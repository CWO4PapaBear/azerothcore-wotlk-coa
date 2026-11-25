from pathlib import Path
import ast,json,re,sqlite3,subprocess,sys,tempfile

import os
root=Path(os.environ["FEATURED_CATALOG_FIXTURE"])
repo=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(repo/'tools/featured_catalog'))
from native_wire import decode_wire
from catalog_worker import records
raw=(root/'builds.tsv').read_text()
assigned=records(raw,decode_wire)
sqlsource=(repo/'src/server/database/Database/Implementation/CharacterDatabase.cpp').read_text()
def statement(name):
    body=re.search(r'PrepareStatement\('+name+r',\s*(.*?)CONNECTION_ASYNC\);',sqlsource,re.S).group(1)
    return ''.join(ast.literal_eval(s) for s in re.findall(r'"(?:[^"\\]|\\.)*"',body))
db=sqlite3.connect(':memory:')
db.create_function('IF',3,lambda condition,a,b:a if condition else b)
db.create_function('JSON_UNQUOTE',1,lambda x:x)
db.executescript('CREATE TABLE area52_build(id TEXT PRIMARY KEY,owner_account INT,archived INT); CREATE TABLE area52_build_revision(build_id TEXT,revision INT,document TEXT,category INT); CREATE TABLE area52_build_archetype(archetype_id INT,build_id TEXT,revision INT); CREATE TABLE area52_build_featured(build_id TEXT,revision INT);')
cpp=['#include "Area52FeaturedRewards.h"','#include <cassert>','int main(){']
results=[]
for archetype,revision,wire in (line.split('\t') for line in raw.splitlines()):
    archetype,revision=int(archetype),int(revision)
    if archetype not in (27,86):continue
    ident=str(archetype)
    db.execute('INSERT INTO area52_build VALUES(?,100,0)',(ident,))
    db.execute('INSERT INTO area52_build_revision VALUES(?,?,?,8)',(ident,revision-1,json.dumps({'wire':'previous'})))
    db.execute('INSERT INTO area52_build_archetype VALUES(?,?,?)',(archetype,ident,revision-1))
    db.execute('INSERT INTO area52_build_featured VALUES(?,?)',(ident,revision-1))
    db.commit()
    def update():
        db.execute('INSERT INTO area52_build_revision VALUES(?,?,?,8)',(ident,revision,json.dumps({'wire':wire})))
        db.execute('UPDATE area52_build_archetype SET revision=? WHERE archetype_id=?',(revision,archetype))
        db.execute('UPDATE area52_build_featured SET revision=? WHERE build_id=?',(revision,ident))
    update();db.rollback()
    assert db.execute(statement('CHAR_SEL_A52_CREATION_DRAFT'),(archetype,)).fetchone()[1]==revision-1
    update();db.commit()
    for name,parameters in [('CHAR_SEL_A52_CREATION_DRAFT',(archetype,)),('CHAR_SEL_A52_BUILD_PUBLIC',(ident,999)),('CHAR_SEL_A52_BUILD_PUBLIC',(ident,100))]:
        row=db.execute(statement(name),parameters).fetchone()
        assert row[1:]==(revision,wire,1),(name,row[1])
    assert db.execute(statement('CHAR_SEL_A52_ACTIVE_REVISION'),(ident,revision-1)).fetchone()[1]==revision-1
    build=decode_wire(bytes.fromhex(wire));items=assigned[archetype]['items']
    cpp+=['{Area52Build::FeaturedRewardItems items{};',
          'assert(Area52Build::ReadFeaturedRewards('+json.dumps(build['Description'])+',items));',
          'assert((items==Area52Build::FeaturedRewardItems{'+','.join(map(str,items))+'}));',
          'assert(Area52Build::FeaturedRewardLevels[12]==20 && Area52Build::FeaturedRewardLevels[13]==60);','}']
    results.append({'archetype':archetype,'revision':revision,'items':items})
cpp.append('}')
with tempfile.TemporaryDirectory() as directory:
    path=Path(directory);(path/'test.cpp').write_text('\n'.join(cpp))
    subprocess.run(['g++','-std=c++20','-I',str(repo/'src/server/coa'),str(path/'test.cpp'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
print('PASS new Draft queries and native reward parser match saved vanity metadata; existing revisions unchanged.')
