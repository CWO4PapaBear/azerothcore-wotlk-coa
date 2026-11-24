from pathlib import Path
import subprocess,time,uuid,json,re
ROOT=Path(__file__).resolve().parents[3]
def main():
    name='area52-eclipse-proc-fixture-'+uuid.uuid4().hex[:10]
    def sql(query, database='fixture'):
        r=subprocess.run(['docker','exec','-i',name,'mysql','-uroot','-N','-B',database],input=query,text=True,capture_output=True)
        if r.returncode:raise RuntimeError('Disconnected SQL fixture query failed: '+str(re.findall(r'ERROR ([0-9]+) \(([^)]+)\)',r.stderr)))
        return r.stdout
    subprocess.run(['docker','run','-d','--name',name,'--network','none','--tmpfs','/var/lib/mysql:rw,size=1073741824','-e','MYSQL_ALLOW_EMPTY_PASSWORD=yes','mysql:8.4'],check=True,capture_output=True)
    try:
        for attempt in range(120):
            ready=subprocess.run(['docker','exec',name,'mysqladmin','--protocol=TCP','-h127.0.0.1','-uroot','ping'],capture_output=True)
            if ready.returncode==0:break
            time.sleep(.5)
        else:raise RuntimeError('Disconnected SQL fixture did not start')
        sql('CREATE DATABASE fixture;', 'mysql')
        sql(json.loads(Path(__file__).with_name('schema.json').read_text())['create_table'])
        baseline='INSERT INTO spell_proc VALUES(-48506,0,7,5,0,0,0,0,2,0,0,0,0,0,0,0);'
        sql(baseline)
        before=sql('SELECT * FROM spell_proc ORDER BY SpellId;')
        migration=(ROOT/'data/sql/updates/pending_db_world/rev_20261124_01_area52_eclipse_talent_procs.sql').read_text()
        sql(migration);once=sql('SELECT * FROM spell_proc ORDER BY SpellId;');sql(migration)
        assert sql('SELECT * FROM spell_proc ORDER BY SpellId;')==once
        assert sql('SELECT SpellFamilyMask0,SpellFamilyMask2,SpellPhaseMask FROM spell_proc WHERE SpellId=-48506;').strip()=='5\t524288\t2'
        assert sql('SELECT SpellFamilyMask2,ProcFlags,SpellPhaseMask,DisableEffectsMask,Chance FROM spell_proc WHERE SpellId=984419;').strip()=='524288\t65536\t1\t3\t100'
        sql('DELETE FROM spell_proc WHERE SpellId=984419; UPDATE spell_proc SET SpellFamilyMask2=0 WHERE SpellId=-48506;')
        assert sql('SELECT * FROM spell_proc ORDER BY SpellId;')==before
        print('PASS exact MySQL schema: proc migration, repeated apply, preserved Earth/Moon columns and exact fixture rollback')
    finally:
        subprocess.run(['docker','rm','-f',name],check=True,capture_output=True)
if __name__=='__main__':main()
