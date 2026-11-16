from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
compiler = shutil.which(os.environ.get('CXX', 'c++'))
if not compiler:
    raise SystemExit('C++20 compiler unavailable')
source = r'''
#include "Area52CertificationRules.h"
#include <boost/property_tree/json_parser.hpp>
#include <cassert>
#include <sstream>
int main()
{
    using namespace Area52Certification;
    std::map<std::uint32_t, TesterScore> scores;
    assert(Leaderboard(scores).empty());
    Credit(scores, 18, "Firstalt");
    Credit(scores, 18, "Secondalt");
    Credit(scores, 20, "Other");
    Credit(scores, 20, "Other");
    Credit(scores, 25, "Third");
    scores.emplace(30, TesterScore{30, "Zero", 0, 0});
    auto board = Leaderboard(scores);
    assert(board.size() == 4 && board[0].Total == 2 && board[0].Name == "Secondalt");
    assert(board[3].Total == 0 && board[3].Name == "Zero" && board[3].Rank == 4);
    assert(board[0].Account == 18 && board[1].Account == 20);
    assert(board[0].Rank == 1 && board[1].Rank == 1 && board[2].Rank == 3);
    for (auto text : {"L|0", "L|-1", "L|1x", "L|4294967296", "C|1|7", "L|"})
        assert(ParseLeaderboardPage(text) == 0);
    assert(ParseLeaderboardPage("L|2") == 2);
    for (auto text : {"", "C|0|7", "C|12|3", "C|-1|7", "C|4294967296|7", "C|12x|7", "C|12|7|7", "C||7"})
        assert(ParseConfirmation(text) == 0);
    assert(ParseConfirmation("C|1234|7") == 1234);
    std::istringstream input(R"({"schema":1,"entries":[
        {"kind":"advancement","id":1,"status":"VERIFIED"},
        {"kind":"advancement","id":2,"status":"VERIFIED","certified":true},
        {"kind":"advancement","id":3,"status":"CERTIFIED"},
        {"kind":"advancement","id":4,"status":"Needs Investigation","certified":true},
        {"kind":"mystic","id":1,"status":"VERIFIED"}]})");
    boost::property_tree::ptree report;
    boost::property_tree::read_json(input, report);
    auto result = ReadEligibility(report);
    assert(result.size() == 4 && !result.at(1) && result.at(2) && result.at(3));
    assert(!result.at(MysticEntryMask | 1));
    assert(ParseConfirmation("C|2147483649|7") == (MysticEntryMask | 1));
    auto duplicate = report.get_child("entries").front();
    report.get_child("entries").push_back(duplicate);
    bool rejected = false;
    try { ReadEligibility(report); } catch (std::runtime_error const&) { rejected = true; }
    assert(rejected);
    report.put("schema", 99);
    rejected = false;
    try { ReadEligibility(report); } catch (std::runtime_error const&) { rejected = true; }
    assert(rejected);
}
'''
with tempfile.TemporaryDirectory() as tmp:
    p = Path(tmp)
    (p / 'test.cpp').write_text(source)
    subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror', '-I' + str(root / 'src/server/coa'),
                    str(p / 'test.cpp'), '-o', str(p / 'test')], check=True)
    subprocess.run([str(p / 'test')], check=True)
print('Certification eligibility, prior certification, fail-closed ledger and confirmation parsing passed.')

import re
import sqlite3
database_source = (root / 'src/server/database/Database/Implementation/CharacterDatabase.cpp').read_text()
block = database_source.split('PrepareStatement(CHAR_SEL_A52_CERTIFICATION_REWARD,', 1)[1].split('CONNECTION_SYNCH);', 1)[0]
query = ''.join(re.findall(r'"([^"\\]*(?:\\.[^"\\]*)*)"', block))
db = sqlite3.connect(':memory:')
db.executescript("""
CREATE TABLE area52_certification_reward_rule (account INTEGER PRIMARY KEY, required_count INTEGER, item INTEGER);
CREATE TABLE area52_certification_reward_delivery (account INTEGER PRIMARY KEY, guid INTEGER, item INTEGER);
CREATE TABLE area52_tester_certification (entry INTEGER PRIMARY KEY, account INTEGER, checks_confirmed INTEGER);
INSERT INTO area52_certification_reward_rule VALUES (18,30,9901073);
""")
db.executemany('INSERT INTO area52_tester_certification VALUES (?,18,7)', [(x,) for x in range(1,30)])
assert db.execute(query, (18,)).fetchone() is None
db.execute('INSERT INTO area52_tester_certification VALUES (2147564739,18,3)')
assert db.execute(query, (18,)).fetchone() is None
db.execute('UPDATE area52_tester_certification SET checks_confirmed=7 WHERE entry=2147564739')
assert db.execute(query, (18,)).fetchone() == (9901073,)
assert db.execute(query, (19,)).fetchone() is None
db.execute('INSERT INTO area52_certification_reward_delivery VALUES (18,123,9901073)')
assert db.execute(query, (18,)).fetchone() is None
print('Reward query: threshold, mixed categories, incomplete checks, account isolation and delivered exclusion passed.')
