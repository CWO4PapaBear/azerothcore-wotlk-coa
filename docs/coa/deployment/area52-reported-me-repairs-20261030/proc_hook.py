from pathlib import Path
import json,os,re,subprocess

ROOT=Path(__file__).resolve().parent
COLUMNS=('SpellId','SchoolMask','SpellFamilyName','SpellFamilyMask0','SpellFamilyMask1','SpellFamilyMask2',
         'ProcFlags','SpellTypeMask','SpellPhaseMask','HitMask','AttributesMask','DisableEffectsMask',
         'ProcsPerMinute','Chance','Cooldown','Charges')
EXPECTED=[978734,1,0,0,0,0,680,1,2,0,0,0,0,100,0,0]

def query(text):
    run=subprocess.run(['docker','exec','-i','ascension-freepick-database','mysql',
        '--defaults-extra-file=/run/secrets/mysql-root.cnf','-BNr','acore_world'],input=text,text=True,capture_output=True)
    if run.returncode:
        code=re.search(r'ERROR \d+ \([^)]+\)',run.stderr)
        raise RuntimeError('Cosmic proc SQL failed: '+(code.group() if code else 'details withheld'))
    return run.stdout.strip()

def schema():
    if query('SHOW CREATE TABLE spell_proc;')!=json.loads((ROOT/'native-provenance.json').read_text())['proc_schema']:
        raise RuntimeError('Cosmic proc schema changed')

def rows():
    return [json.loads(line) for line in query('SELECT JSON_ARRAY('+','.join('`'+c+'`' for c in COLUMNS)+
        ') FROM spell_proc WHERE SpellId=978734;').splitlines()]

def preflight():
    schema()
    if rows():raise RuntimeError('Cosmic proc baseline changed; reconcile existing row')

def saved():
    return Path(os.environ['AREA52_TRANSACTION_DIR'])/'cosmic-proc-before.json'

def backup():
    preflight()
    if saved().exists():
        if json.loads(saved().read_text())!={'before':[],'after':[EXPECTED]}:
            raise RuntimeError('Cosmic backup differs')
    else:
        with saved().open('x') as out:
            json.dump({'before':[],'after':[EXPECTED]},out)
            out.flush();os.fsync(out.fileno())

def transition(reverse=False):
    schema()
    if json.loads(saved().read_text())!={'before':[],'after':[EXPECTED]}:
        raise RuntimeError('Cosmic backup mismatch')
    actual=rows()
    if actual not in ([],[EXPECTED]):raise RuntimeError('Cosmic proc changed; preserve it')
    target=[] if reverse else [EXPECTED]
    if actual==target:return
    guard=' AND '.join('`'+c+'`='+str(v) for c,v in zip(COLUMNS,EXPECTED))
    statements=['CREATE TEMPORARY TABLE cosmic_guard (ok INT NOT NULL);','START TRANSACTION;']
    if reverse:
        statements+=['DELETE FROM spell_proc WHERE '+guard+';',
                     'INSERT INTO cosmic_guard VALUES(IF(ROW_COUNT()=1,1,NULL));']
    else:
        statements+=['INSERT INTO cosmic_guard SELECT IF(COUNT(*)=0,1,NULL) FROM spell_proc '
                     'WHERE SpellId=978734 FOR UPDATE;',
                     (ROOT/'candidate/data/sql/updates/pending_db_world/2026_10_30_01_area52_cosmic_reprisal.sql').read_text()]
    query('\n'.join(statements+['COMMIT;']))
    if rows()!=target:raise RuntimeError('Cosmic proc readback differs; preserve transaction')

def apply():transition()
def rollback():transition(True)
def verify():
    if rows()!=[EXPECTED]:raise RuntimeError('Cosmic proc result differs')

if __name__=='__main__':
    import sys
    {'preflight':preflight,'backup':backup,'apply':apply,'verify':verify,'rollback':rollback}[sys.argv[1]]()
