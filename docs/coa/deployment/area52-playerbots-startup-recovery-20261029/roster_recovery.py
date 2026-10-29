"""Exact owned-roster recovery; no operation on import. Private backups only."""
from pathlib import Path
import hashlib,json,os,re,subprocess,sys
PAYLOAD=Path(os.environ.get('AREA52_PACKAGE_DIR',Path(__file__).resolve().parents[1]))
TRANSACTION=Path(os.environ.get('AREA52_TRANSACTION_DIR','/nonexistent'))
CONTAINER='ascension-freepick-database'
def require(ok,message):
    if not ok:raise RuntimeError(message)
def query(sql):
    r=subprocess.run(['docker','exec','-i',CONTAINER,'mysql','--defaults-extra-file=/run/secrets/mysql-root.cnf','-BNr','acore_characters'],input=sql,text=True,capture_output=True)
    if r.returncode:
        code=re.search(r'ERROR \d+ \([A-Z0-9]+\)',r.stderr)
        raise RuntimeError('Roster recovery SQL failed: '+(code.group() if code else 'details withheld'))
    return r.stdout.strip()
def ident(name):
    require(bool(re.fullmatch(r'\w+',name)),'Unsafe SQL identifier')
    return '`'+name+'`'
def qualified(spec):return ident(spec['database'])+'.'+ident(spec['table'])
def snapshot(spec):
    columns=query("SELECT COLUMN_NAME FROM information_schema.columns WHERE TABLE_SCHEMA='"+spec['database']+"' AND TABLE_NAME='"+spec['table']+"' ORDER BY ORDINAL_POSITION;").splitlines()
    require(bool(columns),'Missing recovery table')
    types=query("SELECT DATA_TYPE,IFNULL(CHARACTER_SET_NAME,'') FROM information_schema.columns WHERE TABLE_SCHEMA='"+spec['database']+"' AND TABLE_NAME='"+spec['table']+"' ORDER BY ORDINAL_POSITION;")
    # Promote FLOAT before textual serialization to retain its stored precision,
    # rather than using the shorter display formatting of a FLOAT column.
    expressions=[]
    for column,kind in zip(columns,types.splitlines()):
        value=ident(column)
        if kind.partition('\t')[0]=='float':value='CAST('+value+' AS DOUBLE)'
        expressions.append("IFNULL(CONCAT('0x',HEX(CAST("+value+" AS BINARY))),'NULL')")
    rows=sorted(query('SELECT '+','.join(expressions)+' FROM '+qualified(spec)+' WHERE '+spec['where']+';').splitlines())
    return {'columns':columns,'types':types.splitlines(),'rows':rows}
def digest(data):return hashlib.sha256(json.dumps(data,sort_keys=True).encode()).hexdigest()
def config():return json.loads((PAYLOAD/'roster-recovery.json').read_text())
def inserts(spec,data):
    statements=[]
    for row in data['rows']:
        values=row.split('\t');require(len(values)==len(data['columns']),'Invalid recovery backup row')
        require(all(v=='NULL' or re.fullmatch(r'0x[0-9A-F]*',v) for v in values),'Invalid recovery backup value')
        encoded=[]
        for value,kind in zip(values,data['types']):
            sqltype,_,charset=kind.partition('\t')
            if value=='NULL':encoded.append(value)
            elif sqltype in ('binary','varbinary','tinyblob','blob','mediumblob','longblob','bit'):encoded.append("''" if value=='0x' else value)
            else:
                charset=charset or 'ascii';require(bool(re.fullmatch(r'\w+',charset)),'Invalid character set')
                encoded.append("''" if value=='0x' else 'CONVERT('+value+' USING '+charset+')')
        statements.append('INSERT INTO '+qualified(spec)+' ('+','.join(ident(c) for c in data['columns'])+') VALUES ('+','.join(encoded)+');')
    return statements
def check_owner(cfg):
    actual=query('SELECT roster_index,account_id FROM acore_playerbots.playerbots_area52_pilot_owner ORDER BY roster_index;')
    require(actual==cfg['owners'],'Recovery ownership changed')
    require(query("SELECT COUNT(*) FROM acore_auth.account a JOIN acore_playerbots.playerbots_area52_pilot_owner p ON p.account_id=a.id WHERE a.username=CONCAT('A52PILOT',p.roster_index) AND p.roster_index IN (0,1);")=='2','Recovery account identity changed')
    accounts=','.join(str(x) for x in cfg['accounts'])
    require(query('SELECT COUNT(*) FROM acore_auth.account_access WHERE id IN ('+accounts+');')=='0','Bot accounts gained elevated access')
    require(query('SELECT COUNT(*) FROM characters WHERE account IN ('+accounts+') AND online<>0;')=='0','Bot characters are online')
def preflight():
    cfg=config();check_owner(cfg)
    for spec in cfg['tables']:
        require(digest(snapshot(spec))==spec['sha256'],'Recovery rows changed: '+spec['database']+'.'+spec['table'])
        engine=query("SELECT ENGINE FROM information_schema.tables WHERE TABLE_SCHEMA='"+spec['database']+"' AND TABLE_NAME='"+spec['table']+"';")
        require(engine=='InnoDB','Recovery requires transactional tables')
    for guard in cfg['empty_guards']:
        require(query('SELECT COUNT(*) FROM '+qualified(guard)+' WHERE '+guard['where']+';')=='0','Unexpected relationship: '+guard['table'])
    print('Exact owned-roster and relationship guards passed; no changes.')
def backup():
    preflight();folder=TRANSACTION/'roster-backup';folder.mkdir(mode=0o700,exist_ok=False)
    for i,spec in enumerate(config()['tables']):
        data=snapshot(spec);require(digest(data)==spec['sha256'],'Recovery changed during backup')
        path=folder/(str(i)+'.json');path.write_text(json.dumps(data));path.chmod(0o600)
def assertion(sql):
    # A duplicate primary key aborts the connection transaction, with no commits.
    return 'INSERT INTO recovery_assertion SELECT 1 WHERE NOT ('+sql+');'
def apply():
    preflight();cfg=config();folder=TRANSACTION/'roster-backup'
    for i,spec in enumerate(cfg['tables']):require(digest(json.loads((folder/(str(i)+'.json')).read_text()))==spec['sha256'],'Missing or changed private backup')
    statements=['CREATE TEMPORARY TABLE recovery_assertion (id INT PRIMARY KEY) ENGINE=InnoDB;','INSERT INTO recovery_assertion VALUES(1);','START TRANSACTION;']
    for spec in cfg['tables']:
        statements.append('DELETE FROM '+qualified(spec)+' WHERE '+spec['where']+';')
        statements.append(assertion('ROW_COUNT()='+str(spec['count'])))
    statements.append('COMMIT;')
    query('\n'.join(statements))
    (TRANSACTION/'roster-removal-complete').write_text('Owned roster removed; exact backups retained.')
    print('Reviewed incorrect roster removed transactionally; private backups retained.')
def verify():
    require((TRANSACTION/'roster-removal-complete').exists(),'Recovery removal receipt missing')
    # The pilot hook separately verifies all newly created characters and roles.
    print('Recovery receipt present; pilot verification checks the replacement roster.')
def rollback():
    cfg=config();folder=TRANSACTION/'roster-backup'
    if not folder.exists():return
    old=[];current=[]
    for i,spec in enumerate(cfg['tables']):
        data=json.loads((folder/(str(i)+'.json')).read_text());require(digest(data)==spec['sha256'],'Recovery backup changed')
        old.append(data);current.append(snapshot(spec))
    if all(digest(a)==digest(b) for a,b in zip(old,current)):return
    if any(data['rows'] for data in current):
        # Keep a valid partially/fully created replacement pool dormant, as the
        # pilot's rollback policy requires. Never overwrite its newer state with
        # the incorrect old roster; the exact original backup remains private.
        check_owner(cfg)
        rows=query("SELECT p.roster_index,c.class,c.race,TRIM(s.data) FROM acore_playerbots.playerbots_area52_pilot_owner p JOIN characters c ON c.account=p.account_id LEFT JOIN character_settings s ON s.guid=c.guid AND s.source='core.area52.bot.pilot' ORDER BY p.roster_index,c.guid;").splitlines()
        require(0<len(rows)<=20,'Unrecognized replacement rows; preserve backups for reconciliation')
        counts={}
        for row in rows:
            fields=row.split('\t');require(len(fields)==4 and all(x.isdigit() for x in fields),'Replacement ownership marker missing')
            faction,cls,race,role=map(int,fields)
            require(faction in (0,1) and cls==10 and role in (1,2,3),'Replacement is not an owned Hero pilot')
            require(race in ((1,3,4,7,11) if faction==0 else (2,5,6,8,10)),'Replacement faction changed')
            key=(faction,role);counts[key]=counts.get(key,0)+1
            require(counts[key]<={1:2,2:2,3:6}[role],'Replacement role quota exceeded')
        (TRANSACTION/'roster-rollback-retained-new-pool.json').write_text(json.dumps({'characters':len(rows),'policy':'New owned Hero pool retained dormant; original mixed-class rows remain in private backup.'}))
        print('Replacement Hero pool retained dormant; original mixed-class backups preserved without overwriting newer records.')
        return
    statements=['START TRANSACTION;']
    for spec,data in zip(cfg['tables'],old):
        statements.extend(inserts(spec,data))
    statements.append('COMMIT;');query('\n'.join(statements))
    for spec in cfg['tables']:require(digest(snapshot(spec))==spec['sha256'],'Recovery restoration differs')
    print('Original owned roster restored from exact private backups.')
if __name__=='__main__':
    require(len(sys.argv)==2 and sys.argv[1] in ('preflight','backup','apply','verify','rollback'),'Invalid phase')
    globals()[sys.argv[1]]()
