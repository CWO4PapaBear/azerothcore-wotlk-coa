from pathlib import Path
import subprocess,time,uuid,json,re
ROOT=Path(__file__).resolve().parents[3]
HERE=Path(__file__).parent
name='area52-wind-sql-'+uuid.uuid4().hex[:10]
def sql(text):
    result=subprocess.run(['docker','exec','-i',name,'mysql','-uroot','-N','-B','acore_world'],input=text,text=True,capture_output=True)
    if result.returncode:
        error=re.search(r'ERROR (\d+) \(([^)]+)\)',result.stderr)
        raise RuntimeError('Offline SQL fixture failed: '+(error.group(0) if error else 'unclassified'))
    return result.stdout.strip()
def main():
    subprocess.run(['docker','run','-d','--name',name,'--network','none','--tmpfs','/var/lib/mysql:rw','-e','MYSQL_ALLOW_EMPTY_PASSWORD=yes','-e','MYSQL_DATABASE=acore_world','mysql:8.4'],check=True,capture_output=True)
    try:
        for _ in range(90):
            logs=subprocess.run(['docker','logs',name],capture_output=True,text=True)
            r=subprocess.run(['docker','exec',name,'mysql','-uroot','acore_world','-e','SELECT 1'],capture_output=True)
            if 'MySQL init process done. Ready for start up.' in logs.stdout+logs.stderr and r.returncode==0:break
            time.sleep(1)
        else:raise RuntimeError('Offline SQL startup timed out')
        sql(json.loads(HERE.joinpath('schema.json').read_text())['sql'])
        sql("INSERT INTO spell_script_names (spell_id,ScriptName) VALUES (978232,'unrelated_preserved'); INSERT INTO spell_group (id,spell_id) VALUES (1101,1490),(1101,51726),(1101,60431); INSERT INTO spell_group_stack_rules (group_id,stack_rule) VALUES (1101,3);")
        baseline=sql('SELECT * FROM spell_script_names ORDER BY spell_id,ScriptName; SELECT * FROM spell_group ORDER BY id,spell_id; SELECT * FROM spell_proc ORDER BY SpellId; SELECT * FROM spell_group_stack_rules ORDER BY group_id;')
        migrations=[ROOT/'data/sql/updates/pending_db_world'/n for n in ('2026_11_25_03_area52_wind_rager_deterrence.sql','2026_11_25_04_area52_wind_rager_procs.sql')]
        for _ in range(2):
            for path in migrations:sql(path.read_text())
            assert sql('SELECT COUNT(*) FROM spell_script_names WHERE spell_id=978232;')=='2'
            assert sql('SELECT COUNT(*) FROM spell_script_names WHERE spell_id=19263;')=='1'
            assert sql('SELECT Chance,Cooldown,DisableEffectsMask,SchoolMask FROM spell_proc WHERE SpellId=978232;')=='10\t6000\t6\t1'
            assert sql('SELECT COUNT(*) FROM spell_group WHERE id=1101;')=='4'
            assert sql('SELECT ProcFlags,SpellPhaseMask,AttributesMask,Charges FROM spell_proc WHERE SpellId=997084;')=='65536\t1\t8\t1'
        sql(json.loads(HERE.joinpath('rollback.json').read_text())['sql'])
        restored=sql('SELECT * FROM spell_script_names ORDER BY spell_id,ScriptName; SELECT * FROM spell_group ORDER BY id,spell_id; SELECT * FROM spell_proc ORDER BY SpellId; SELECT * FROM spell_group_stack_rules ORDER BY group_id;')
        assert restored==baseline
        print('PASS actual MySQL8.4 schema: combined migrations twice, exact native proc fields, additive bindings/group, unchanged unrelated rows, exact baseline rollback')
    finally:subprocess.run(['docker','rm','-f',name],check=True,capture_output=True)
if __name__=='__main__':main()
