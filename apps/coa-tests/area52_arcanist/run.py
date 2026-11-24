from pathlib import Path
import re,sqlite3,struct,subprocess,tempfile,sys

ROOT=Path(__file__).resolve().parents[3]

def extract(source,signature):
    start=source.index(signature)
    end=source.index('{',start)+1
    depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}')
        end+=1
    return source[start:end].replace(' override','')

def main():
    inc=(ROOT/'src/server/coa/Area52Arcanist.inc').read_text()
    mystic=(ROOT/'src/server/coa/AscensionMysticEnchant.cpp').read_text()
    unit=(ROOT/'src/server/game/Entities/Unit/Unit.cpp').read_text()
    code=Path(__file__).with_name('harness.cpp').read_text()
    methods={
        'ELIGIBLE':extract(inc,'bool Eligible('),
        'SYNC':extract(inc,'void Synchronize('),
        'CONNECTED':extract(inc,'bool Connected('),
        'CAST':extract(inc,'void OnSpellBeforeEffects('),
        'METADATA':extract(inc,'void OnLoadSpellCustomAttr('),
        'POTENTIAL':extract(inc[inc.index('class aura_area52_arcanist_potential'):],'void Apply('),
        'REMOVE':extract(inc[inc.index('class aura_area52_arcanist_resonance'):],'void Remove('),
        'UNEQUIP':extract(inc[inc.index('class aura_area52_arcanist_equipment'):],'void Remove(').replace('void Remove(','void Unequip(',1),
        'TARGETS':extract(inc,'void Targets('),
        'HIT':extract(inc,'void Hit('),
        'PVP':extract(unit,'float Unit::GetAscensionPvpTuningDamageMultiplier(').replace('Unit::','Player::'),
        'RESTORE':extract(mystic,'void Synchronize(Player* player)'),
        'SAVE':extract((ROOT/'src/server/game/Spells/Auras/SpellAuras.cpp').read_text(),'bool Aura::CanBeSaved() const'),
    }
    for name,source in methods.items():code=code.replace('@'+name+'@',source)
    with tempfile.TemporaryDirectory() as directory:
        path=Path(directory);(path/'test.cpp').write_text(code)
        subprocess.run(['c++','-std=c++20','-Wall','-Wextra','-I',str(ROOT/'src/server/coa'),str(path/'test.cpp'),'-o',str(path/'test')],check=True)
        subprocess.run([str(path/'test')],check=True)
        sys.path.insert(0,str(ROOT/'apps/coa-tests'))
        from coa_talent_catalog import STUBS
        for name,content in STUBS.items():(path/name).write_text(content)
        catalog=Path(__file__).with_name('catalog.cpp')
        includes=[path,ROOT/'src/server/coa',ROOT/'src/server/shared/DataStores',ROOT/'src/common']
        subprocess.run(['c++','-std=c++20',*['-I'+str(p) for p in includes],str(catalog),str(ROOT/'src/server/coa/AscensionMysticEnchantCatalog.cpp'),str(ROOT/'src/server/shared/DataStores/ClientDBC.cpp'),'-o',str(path/'catalog')],check=True)
        subprocess.run([str(path/'catalog'),'/data/dbc/MysticEnchant.dbc'],check=True)
    sql=(ROOT/'data/sql/updates/pending_db_world/rev_20261124_02_area52_arcanist.sql').read_text()
    database=sqlite3.connect(':memory:')
    database.execute('CREATE TABLE spell_script_names (spell_id INTEGER,ScriptName TEXT,PRIMARY KEY(spell_id,ScriptName))')
    database.execute("INSERT INTO spell_script_names VALUES(984117,'unrelated')")
    for _ in range(2):
        database.executescript(sql)
        assert database.execute('SELECT COUNT(*) FROM spell_script_names').fetchone()[0]==11
    database.executescript(sql[:sql.index('INSERT INTO')])
    assert database.execute('SELECT * FROM spell_script_names').fetchall()==[(984117,'unrelated')]
    database.executescript(sql)
    spell=(ROOT/'src/server/game/Spells/Spell.cpp').read_text()
    assert spell.index('sScriptMgr->OnSpellBeforeEffects(this, m_caster, m_spellInfo);')>spell.index('    SendSpellCooldown();',spell.index('void Spell::cast('))
    for name in ('aura_area52_arcanist_equipment','aura_area52_arcanist_potential','aura_area52_arcanist_resonance','spell_area52_arcanist_meteor'):
        assert 'RegisterSpellScript('+name+');' in mystic and name in sql
    assert 'new area52_arcanist_casts();' in mystic and 'new area52_arcanist_metadata();' in mystic
    assert 'if (spell == Area52Arcanist::Enchant)' in mystic and 'if (Area52Arcanist::Connected())' in mystic
    assert 'Area52Arcanist::Synchronize(player);' in extract(mystic,'void Synchronize(Player* player)')
    assert mystic.count('if (spell == Area52Arcanist::Meteor ||')==2
    b=Path('/data/dbc/Spell.dbc').read_bytes();_,count,fields,size,_=struct.unpack_from('<4s4I',b)
    records={}
    wanted={984117,984118,984122,984126,984130,984138,984267,984394,984266,984268,984406,984407,984408,984409}
    for index in range(count):
        row=struct.unpack_from('<234I',b,20+index*size)
        if row[0] in wanted:records[row[0]]=row
    assert set(records)==wanted
    assert records[984138][49]==4 and records[984138][40]==29
    for id in (984118,984122,984126,984130):
        row=records[id]
        assert row[212]==5 and row[92]==14 and row[117]==984138 and row[80]==80
    for id in (984267,984394,984266,984268):assert records[id][40]==31
    for id,bonus in ((984406,60),(984407,50),(984408,40),(984409,60)):
        row=records[id];assert row[80]+1==bonus and row[81]+1==10 and row[40]==1
    print('PASS actual Arcanist cast, cycle, cooldown reset, cleanup, connection readiness, metadata and PvP functions; actual Mystic Synchronize in both saved-state load orders; migration repeat/rollback; original helper target/radius/duration data. No gameplay or live SQL.')

if __name__=='__main__':main()
