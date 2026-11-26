from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import struct
import sys
import tempfile
import time

from catalog_summary import selection, transform
SUMMARY="DBFilesClient/CharacterCreationArchetypes.dbc"

DEFAULT=4294967295
CATEGORIES=(17,18,23,31)
QUERY="""SELECT a.archetype_id,r.revision,JSON_UNQUOTE(JSON_EXTRACT(r.document,'$.wire'))
FROM area52_build_archetype a JOIN area52_build b ON b.id=a.build_id AND b.archived=0
JOIN area52_build_revision r ON r.build_id=a.build_id AND r.revision=a.revision AND r.category=8
JOIN area52_build_featured f ON f.build_id=a.build_id AND f.revision=a.revision
ORDER BY a.archetype_id;"""

def sha(data):
    return hashlib.sha256(data).hexdigest()

def section(description,name,count):
    marker='###'+name+'\n'
    if marker not in description:
        return None
    if description.count(marker)!=1:
        raise ValueError('Duplicate metadata section')
    start=description.index(marker)
    if start and description[start-1]!='\n':
        raise ValueError('Invalid metadata boundary')
    line=description[start+len(marker):].split('\n',1)[0]
    if not re.fullmatch(r'1:\d+(?:,\d+)*',line):
        raise ValueError('Invalid metadata version or encoding')
    values=[int(value) for value in line[2:].split(',')]
    if len(values)!=count or any(value>DEFAULT for value in values):
        raise ValueError('Invalid metadata values')
    return values

def dbc(data,fields):
    magic,count,width,size,strings=struct.unpack_from('<4s4I',data)
    if (magic,width,size)!=(b'WDBC',fields,fields*4) or len(data)!=20+count*size+strings:
        raise ValueError('DBC layout mismatch')
    return [list(struct.unpack_from('<'+'I'*fields,data,20+i*size)) for i in range(count)]

def records(raw,decode):
    result={}
    if len(raw)>32*1024*1024:
        raise ValueError('Catalog query limit exceeded')
    for line in raw.splitlines():
        archetype,revision,wire=line.split('\t')
        archetype,revision=int(archetype),int(revision)
        if not 1<=archetype<=65535 or revision<1 or archetype in result:
            raise ValueError('Invalid assigned revision')
        build=decode(bytes.fromhex(wire))
        if build['Category']!=8:
            raise ValueError('Non-Featured record')
        description=build['Description']
        ids=re.findall(r'(?:^|\n)###AREA52_ARCHETYPE_ID\n(\d+)(?:\n|$)',description)
        if ids!=[str(archetype)]:
            raise ValueError('Assignment metadata mismatch')
        result[archetype]={'revision':revision,'summary':selection(description),'items':section(description,'AREA52_FEATURED_VANITY',14),
            'effects':section(description,'AREA52_FEATURED_PREVIEW',4),
            'incarnations':section(description,'AREA52_FEATURED_INCARNATIONS',4),
            'weapons':section(description,'AREA52_FEATURED_WEAPONS',1)}
        if result[archetype]['weapons'] is not None and result[archetype]['weapons'][0] not in (0,1,2,3):
            raise ValueError('Invalid preview weapon visibility')
    return result

def render(base,assigned,incarnations,item_data):
    rows=dbc(base,28)
    items={row[0]:row[6] for row in dbc(item_data,8)}
    appearance={row['appearance_id']:row for row in incarnations}
    category_by_creature={row['creature_id']:row['category'] for row in incarnations}
    available={row[1] for row in rows}
    if set(assigned)-available:
        raise ValueError('Archetype has no client preview rows')
    slots=({1},{3},{5,20},{10},{8},{9},{6},{16},{7},{13,17,21},{13,14,22,23},{15,25,26})
    for record in assigned.values():
        if record['items'] is not None:
            for index,item in enumerate(record['items'][:12]):
                if item and items.get(item) not in slots[index]:
                    raise ValueError('Preview equipment slot mismatch')
            if items.get(record['items'][9])==17 and record['items'][10]:
                raise ValueError('Two-handed equipment conflict')
        for index,value in enumerate(record['incarnations'] or [DEFAULT]*4):
            if value not in (0,DEFAULT) and (value not in appearance or appearance[value]['category']!=CATEGORIES[index]):
                raise ValueError('Incarnation category mismatch')
    output=bytearray(base)
    for index,row in enumerate(rows):
        record=assigned.get(row[1])
        if not record:
            continue
        if record['items'] is not None:
            row[16:28]=record['items'][:12]
        mode=(record.get('weapons') or [0])[0]
        if mode not in (0,1,2,3):
            raise ValueError('Invalid preview weapon visibility')
        if mode in (2,3):
            row[25:27]=[0,0]
        if mode in (1,3):
            row[27]=0
        effects=record['effects'] or [DEFAULT]*4
        if effects[0]!=DEFAULT:
            row[13]=effects[0]
        rewards=record['incarnations']
        category=category_by_creature.get(row[14])
        if rewards is not None and category in CATEGORIES:
            reward=rewards[CATEGORIES.index(category)]
            if reward!=DEFAULT:
                row[14]=appearance[reward]['creature_id'] if reward else 0
        elif rewards is not None and any(value not in (0,DEFAULT) for value in rewards):
            raise ValueError('Incarnation scene category requires an explicit mapping')
        struct.pack_into('<28I',output,20+index*112,*row)
    lines=['Area52FeaturedPreviewCatalog = {']
    for archetype,record in sorted(assigned.items()):
        effects=record['effects'] or [DEFAULT]*4
        lines.append('    ['+str(archetype)+']={'+str(effects[2])+','+str(effects[3])+'},')
    lines.append('}')
    return bytes(output),('\n'.join(lines)+'\n').encode()

def pinned(path,digest):
    value=Path(path).read_bytes()
    if sha(value)!=digest:
        raise ValueError('Approved catalog baseline drift')
    return value

PRICING_QUERY = "SELECT ItemID,DonationPointCost,SeasonalPointCost,BazaarTokenCost FROM acore_world.coa_client_vanity_collection ORDER BY ItemID;"


def render_prices(data, snapshot):
    magic, count, fields, size, strings = struct.unpack_from('<4s4I', data)
    if (magic, fields, size) != (b'WDBC', 76, 308) or len(data) != 20 + count * size + strings:
        raise ValueError('Vanity price catalog layout mismatch')
    prices = {}
    for line in snapshot.splitlines():
        values = line.split('\t')
        if len(values) != 4 or any(not v.isdecimal() for v in values):
            raise ValueError('Invalid price row')
        item, dp, sp, bt = map(int, values)
        if item in prices or not item or max(dp, sp, bt) > 5000000:
            raise ValueError('Invalid or duplicate price')
        prices[item] = (dp, sp, bt)
    out = bytearray(data)
    for i in range(count):
        offset = 20 + i * size
        item = struct.unpack_from('<I', data, offset + 4)[0]
        if item in prices:
            struct.pack_into('<III', out, offset + 16, *prices[item])
    return bytes(out)


def wardrobe_sets(rewards):
    sets_lua = ['Area52FeaturedSetItems = {']
    for archetype, reward in sorted(rewards.items()):
        if reward['items'] is not None:
            slots = (1,3,5,10,8,9,6,15,7,16,17,18)
            members = [f'[{slot}]={item}' for slot,item in zip(slots,reward['items'][:12]) if item]
            sets_lua.append(f'[{90000+archetype}]={{' + ','.join(members) + '},')
    sets_lua.append('}')
    return ('\n'.join(sets_lua)+'\n').encode()


def cycle(config,query,decode):
    inputs={name:pinned(config[name]['path'],config[name]['sha256']) for name in ('details','items','incarnations','glue','local_glue')}
    snapshot=query(QUERY)
    pricing=query(PRICING_QUERY) if config.get("vanity_prices") else None
    assigned=records(snapshot,decode)
    details,catalog=render(inputs['details'],assigned,json.loads(inputs['incarnations'])['records'],inputs['items'])
    glue=inputs['glue']+b'\n'+catalog+b'\n'+inputs['local_glue']
    files={'DBFilesClient/CharacterCreationArchetypeDetails.dbc':details,'Interface/GlueXML/CharacterCreate.lua':glue}
    if 'rewards' in config:
        from catalog_rewards import render as render_rewards
        reward_inputs={name:pinned(entry['path'],entry['sha256']) for name,entry in config['rewards'].items()}
        sets,vanity,rewards=render_rewards(reward_inputs['sets'],reward_inputs['vanity'],reward_inputs['appearances'],assigned,
            json.loads(reward_inputs['mounts'])['kinds'],json.loads(reward_inputs['defaults'])['bundles']['Bundles'])
        files['DBFilesClient/ItemSet.dbc']=sets
        files['DBFilesClient/VanityCollection.dbc']=vanity

        files['Interface/AddOns/Ascension_AppearanceUI/FeaturedSetItems.lua']=wardrobe_sets(rewards)
        inputs.update({'rewards_'+name:data for name,data in reward_inputs.items()})
    if pricing is not None:
        files['DBFilesClient/VanityCollection.dbc'] = render_prices(files['DBFilesClient/VanityCollection.dbc'], pricing)
        inputs['vanity_prices'] = pricing.encode('utf-8')
    base_files={}
    overrides={i:r['summary'] for i,r in assigned.items() if r['summary'] is not None}
    if overrides and 'summary_catalogs' not in config:
        raise ValueError('Both summary catalogs must be configured')
    if 'summary_catalogs' in config:
        summaries=config['summary_catalogs']
        for key,outputs in [('realm',files),('base',base_files)]:
            entry=summaries[key]
            original=pinned(entry['path'],entry['sha256'])
            inputs['summary_'+key]=original
            outputs[SUMMARY]=transform(original,overrides)[0]
    manifest={'schema':1,'baseline':{name:sha(data) for name,data in inputs.items()},
        'files':{name:sha(data) for name,data in files.items()},'revisions':{str(key):value['revision'] for key,value in sorted(assigned.items())},
        'publication':'pending','dll_changed':False}
    if base_files:
        manifest['base_files']={name:sha(data) for name,data in base_files.items()}
        manifest['baseline']['base_archive']=summaries['archive']['sha256']
    if 'archive' in config:
        pinned(config['archive']['path'],config['archive']['sha256'])
        manifest['baseline']['archive']=config['archive']['sha256']
    encoded=json.dumps(manifest,sort_keys=True,separators=(',',':')).encode()
    identity=sha(encoded)
    root=Path(config['outbox']);root.mkdir(parents=True,exist_ok=True)
    target=root/identity
    exports=dict(files)
    exports.update({"base/"+name:data for name,data in base_files.items()})
    if target.exists():
        if (target/'release-request.json').read_bytes()!=encoded or any(sha((target/name).read_bytes())!=sha(data) for name,data in exports.items()):
            raise ValueError('Immutable export drift')
        if 'archive' in config:
            receipt=json.loads((target/'patch-E.catalog.json').read_text())
            if sha((target/'patch-E.MPQ').read_bytes())!=receipt['candidate_sha256']:
                raise ValueError('Immutable archive drift')
        return identity
    if query(QUERY)!=snapshot or (pricing is not None and query(PRICING_QUERY)!=pricing):
        raise ValueError('Assignments changed during export; retry next cycle')
    temporary=Path(tempfile.mkdtemp(prefix='.staging-',dir=root))
    for name,data in exports.items():
        path=temporary/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)
    (temporary/'release-request.json').write_bytes(encoded)
    if 'archive' in config:
        from catalog_archive import stage
        stage(temporary,config['archive']['path'],config['archive']['sha256'],temporary/'patch-E.MPQ',config['mpq_tools'])
        if base_files:
            entry=summaries['archive']
            stage(temporary,entry['path'],entry['sha256'],temporary/'patch-M.MPQ',config['mpq_tools'],base=True)
    temporary.rename(target)
    return identity
