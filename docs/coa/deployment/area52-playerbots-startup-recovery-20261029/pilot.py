"""Area52 pilot configuration/bootstrap hook. No action on import.

Rollback restores configuration, leaving the isolated owned pool/database dormant
for retry. It never deletes characters or restores a shared human database.
"""
from pathlib import Path
import hashlib,json,os,re,subprocess,sys

PRIVATE=Path('/home/dml/games/ascension-freepick-test/private')
PAYLOAD=Path(os.environ.get('AREA52_PACKAGE_DIR',Path(__file__).resolve().parents[1]))
TRANSACTION=Path(os.environ.get('AREA52_TRANSACTION_DIR','/nonexistent'))
DATABASE='acore_playerbots'
CONTAINER='ascension-freepick-database'

def sha(data):return hashlib.sha256(data).hexdigest()
def require(value,message):
    if not value:raise RuntimeError(message)
def config():return json.loads((PAYLOAD/'pilot-config.json').read_text())
def query(database,statement):
    result=subprocess.run(['docker','exec','-i',CONTAINER,'mysql','--defaults-extra-file=/run/secrets/mysql-root.cnf','-BNr',database],input=statement,text=True,capture_output=True)
    if result.returncode:
        code=re.search(r'ERROR \d+ \([A-Z0-9]+\)',result.stderr)
        raise RuntimeError('Pilot database operation failed: '+(code.group() if code else 'details withheld'))
    return result.stdout.strip()
def literal(value):return "'"+value.replace('\\','\\\\').replace("'","''")+"'"
def values(text):
    result={}
    for line in text.splitlines():
        match=re.match(r'^\s*([A-Za-z][A-Za-z0-9_.]*)\s*=\s*(.*?)\s*$',line)
        if match:
            require(match[1] not in result,'Duplicate configuration key: '+match[1])
            result[match[1]]=match[2].strip('"')
    return result
def render(original,overrides):
    keys=set(overrides)
    lines=[]
    for line in original.splitlines():
        match=re.match(r'^\s*([A-Za-z][A-Za-z0-9_.]*)\s*=',line)
        if not match or match[1] not in keys:lines.append(line)
    lines+=['','# Area52 reusable twenty-bot pilot (managed package settings)']
    for key,value in overrides.items():
        require('\n' not in value and '\r' not in value,'Invalid configuration value')
        lines.append(key+' = '+(value if re.fullmatch(r'\d+',value) else '"'+value.replace('"','\\"')+'"'))
    return ('\n'.join(lines)+'\n').encode()
def connection(text):
    parts=values(text)['LoginDatabaseInfo'].split(';')
    require(len(parts)==5,'Unsupported auth connection format')
    return parts
def installed_config():
    path=Path(config()['config_path'])
    require(path.resolve().is_relative_to(PRIVATE) and path.name=='worldserver.conf','Invalid private config target')
    return path
def exists():return query('mysql',"SELECT COUNT(*) FROM information_schema.schemata WHERE schema_name='acore_playerbots';")=='1'
def marker():
    return query(DATABASE,'SELECT fingerprint FROM area52_pilot_package WHERE id=1;')
def verify_static():
    expected=json.loads((PAYLOAD/'pilot-data.json').read_text())
    for table,columns,rows in expected['tables']:
        actual=query(DATABASE,f"SELECT {columns} FROM {table} ORDER BY {columns};").splitlines()
        require(actual==rows,'Pilot authored data changed: '+table)
def validate_module_configs(cfg):
    for name in cfg.get('absent_module_configs',[]):
        candidate=Path(name)
        require(not candidate.exists() and not candidate.is_symlink(),'Unexpected module configuration could override the pilot')
    for record in cfg.get('required_module_configs',[]):
        module_path=Path(record['path'])
        require(module_path.is_file() and sha(module_path.read_bytes())==record['sha256'],'Required module configuration changed')
        module_values=values(module_path.read_text())
        for key,value in record['required_values'].items():
            require(module_values.get(key)==value,'Required realm mode differs: '+key)

def preflight():
    cfg=config();path=installed_config()
    validate_module_configs(cfg)
    require(sha(path.read_bytes())==cfg['config_sha256'],'Realm configuration drifted')
    data=values(path.read_text())
    require(data.get('MaxPlayerLevel')=='80' and data.get('Expansion')=='2','Pilot requires the reviewed level80/Wrath realm')
    parts=connection(path.read_text())
    hosts=query('mysql','SELECT Host FROM mysql.user WHERE User='+literal(parts[2])+';').splitlines()
    require(len(hosts)==1,'Auth database principal is ambiguous')
    if exists():
        require(marker()==cfg['sql_fingerprint'],'Existing bot database is not this reviewed reusable pool')
        verify_static()
    else:
        require(query('acore_auth',"SELECT COUNT(*) FROM account WHERE username IN ('A52PILOT0','A52PILOT1');")=='0','Reserved bot accounts already exist without ownership')
        for db,tables in cfg['new_support_tables'].items():
            for table in tables:
                require(query('mysql','SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='+literal(db)+' AND table_name='+literal(table)+';')=='0','New support table already exists: '+table)
    print('Pilot configuration/database prerequisites passed; no changes.')
def backup():
    preflight()
    folder=TRANSACTION/'pilot-backup';folder.mkdir(mode=0o700,exist_ok=False)
    (folder/'worldserver.conf').write_bytes(installed_config().read_bytes())
    (folder/'state.json').write_text(json.dumps({'database_existed':exists(),'rollback':'Restore config; retain owned dormant pool data for retry.'}))
def apply():
    cfg=config();path=installed_config()
    validate_module_configs(cfg)
    require(sha(path.read_bytes())==cfg['config_sha256'],'Configuration changed after backup')
    original=path.read_text();parts=connection(original)
    if not exists():
        query('mysql','CREATE DATABASE acore_playerbots CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;')
        for item in cfg['sql']:
            data=(PAYLOAD/item['artifact']).read_bytes()
            require(sha(data)==item['sha256'],'SQL artifact changed')
            query(item['database'],data.decode('utf-8-sig'))
        query(DATABASE,'CREATE TABLE area52_pilot_package (id TINYINT PRIMARY KEY,fingerprint CHAR(64) NOT NULL) ENGINE=InnoDB;')
        query(DATABASE,'INSERT INTO area52_pilot_package VALUES (1,'+literal(cfg['sql_fingerprint'])+');')
    require(marker()==cfg['sql_fingerprint'],'Bot schema is incomplete or belongs to another package')
    verify_static()
    hosts=query('mysql','SELECT Host FROM mysql.user WHERE User='+literal(parts[2])+';').splitlines()
    require(len(hosts)==1,'Auth database principal changed')
    query('mysql','GRANT SELECT,INSERT,UPDATE,DELETE ON acore_playerbots.* TO '+literal(parts[2])+'@'+literal(hosts[0])+';')
    parts[4]=DATABASE
    overrides=values((PAYLOAD/'pilot-overrides.conf').read_text())
    overrides.update({'AiPlayerbot.Enabled':'1','AiPlayerbot.RandomBotAutologin':'1','DungeonFinder.MaxExpansion':'2','PlayerbotsDatabaseInfo':';'.join(parts)})
    candidate=render(original,overrides)
    (TRANSACTION/'pilot-installed-config.sha256').write_text(sha(candidate))
    temporary=path.with_name(path.name+'.pilot-tmp');temporary.write_bytes(candidate);os.chmod(temporary,0o600);temporary.replace(path)
def verify():
    validate_module_configs(config())
    require(sha(installed_config().read_bytes())==(TRANSACTION/'pilot-installed-config.sha256').read_text(),'Installed pilot config changed')
    verify_static()
    require(query(DATABASE,'SELECT COUNT(*) FROM playerbots_area52_pilot_owner;')=='2','Pilot must own exactly two accounts')
    require(query('acore_auth',"SELECT COUNT(*) FROM account a JOIN acore_playerbots.playerbots_area52_pilot_owner p ON p.account_id=a.id WHERE a.username=CONCAT('A52PILOT',p.roster_index) AND p.roster_index IN (0,1);")=='2','Pilot account identity mismatch')
    require(query('acore_characters','SELECT COUNT(*) FROM characters c JOIN acore_playerbots.playerbots_area52_pilot_owner p ON p.account_id=c.account WHERE c.class=10;')=='20','Pilot must contain twenty Hero characters')
    roles=query('acore_characters',"SELECT p.roster_index,TRIM(s.data),COUNT(*) FROM characters c JOIN acore_playerbots.playerbots_area52_pilot_owner p ON p.account_id=c.account LEFT JOIN character_settings s ON s.guid=c.guid AND s.source='core.area52.bot.pilot' GROUP BY p.roster_index,TRIM(s.data) ORDER BY p.roster_index,TRIM(s.data);").splitlines()
    require(roles==[f'{faction}\t{role}\t{count}' for faction in (0,1) for role,count in ((1,2),(2,2),(3,6))],'Pilot roster must be2 tanks/2 healers/6 DPS per account')
    print('Pilot config, authored data and twenty owned Hero characters verified.')
def rollback():
    saved=TRANSACTION/'pilot-backup/worldserver.conf'
    require(saved.is_file(),'Missing pilot configuration backup')
    path=installed_config();receipt=TRANSACTION/'pilot-installed-config.sha256'
    if receipt.exists():
        require(sha(path.read_bytes()) in (receipt.read_text(),sha(saved.read_bytes())),'Configuration edited since pilot apply; refusing overwrite')
        path.write_bytes(saved.read_bytes())
    else:require(sha(path.read_bytes())==sha(saved.read_bytes()),'Unexpected configuration change before apply')
    print('Pilot configuration restored. Owned bot database/roster retained dormant for retry; shared player data untouched.')
if __name__=='__main__':
    require(len(sys.argv)==2 and sys.argv[1] in ('preflight','backup','apply','verify','rollback'),'Invalid phase')
    globals()[sys.argv[1]]()
