from pathlib import Path
import struct,json,hashlib
T=Path(__file__).resolve().parent
raw=(T/'Spell.dbc').read_bytes();b=bytearray(raw);_,n,f,z,s=struct.unpack_from('<4s4I',raw)
one={986203,986201,92840,92843};two={986202,92839,92842};seen=set()
for i in range(n):
 start=20+i*z;key=struct.unpack_from('<I',raw,start)[0]
 if key in one|two:
  assert struct.unpack_from('<3i',raw,start+68*4)==(-1,0,0)
  struct.pack_into('<2i',b,start+68*4,2,0xA091 if key in one else 0x1562);seen.add(key)
assert seen==one|two
# A separately owned, unrestricted passive uses the same rating pipeline as Iron Will.
template_id = 49410  # Forceful Deflection: Strength to parry rating
path_helper_id = 13977921
records = memoryview(raw)[20:20+n*z]
ids = [struct.unpack_from('<I', records, offset)[0] for offset in range(0, len(records), z)]
assert path_helper_id > max(ids) and ids.count(template_id) == 1
start = ids.index(template_id) * z
helper = list(struct.unpack_from('<234i', records, start))
assert helper[68:71] == [-1, 0, 0] and helper[71:74] == [6, 0, 0]
assert (helper[80], helper[95], helper[110], helper[113]) == (24, 220, 8, 0)
helper[0] = path_helper_id
helper[80] = 49  # Base points + one die side = 50 percent.
assert helper[74] == 1
name = b'Path of Strength: Parry Rating\0'
helper[136] = s
b[20+n*z:20+n*z] = struct.pack('<234i', *helper)
b.extend(name)
struct.pack_into('<I', b, 4, n + 1)
struct.pack_into('<I', b, 16, s + len(name))
# Update the English extended tooltip only, retaining its other details and links.
caster_text = {
    84866: ('and |cFFFFFFFFSpell Power|r gained from items and effects is doubled.',
            'and each 1 |cFFFFFFFFSpell Power|r from items and effects grants 2 spell damage power and 1 healing power.'),
    84867: ('and each point of |cFFFFFFFFSpell Power|r now also increases your |cFFFFFFFFHealing Power|r.',
            'and your |cFFFFFFFFHealing Power|r from Spell Power is doubled. Each 1 Spell Power grants 2 healing power and 1 spell damage power.'),
}
pool_start = 20 + (n + 1) * z
for spell_id, (before, after) in caster_text.items():
    field = 20 + ids.index(spell_id) * z + 170 * 4
    old_offset = struct.unpack_from('<I', b, field)[0] + pool_start
    tooltip = bytes(b[old_offset:b.index(0, old_offset)]).decode()
    assert tooltip.count(before) == 1
    struct.pack_into('<I', b, field, len(b) - pool_start)
    b.extend(tooltip.replace(before, after).encode() + bytes(1))
struct.pack_into('<I', b, 16, len(b) - pool_start)
# Private temporary-enchant IDs must fit the 16-bit visible equipment field.
enchant_source = T.parent / 'marksman-leveling-20261103/effective-dbc/SpellItemEnchantment.dbc'
enchants = bytearray(enchant_source.read_bytes())
_, ec, ef, ez, es = struct.unpack_from('<4s4I', enchants)
assert ef == 38 and len(enchants) == 20 + ec * ez + es
enchant_offsets = {struct.unpack_from('<I', enchants, off)[0]: off for off in range(20, 20 + ec * ez, ez)}
child_rows = bytearray()
enchant_rows = bytearray()
for index, child_id in enumerate((36744, 36753, 36754, 36761, 36758, 36760, 36762)):
    child = list(struct.unpack_from('<234i', records, ids.index(child_id) * z))
    assert child[71] == 54 and child[209] == 0
    entry = list(struct.unpack_from('<38I', enchants, enchant_offsets[child[110]]))
    assert entry[2] == 5 and entry[11] == 38
    entry[0] = 60000 + index
    assert entry[0] not in enchant_offsets
    child[0] = 13977922 + index
    child[110] = entry[0]
    child_rows.extend(struct.pack('<234i', *child))
    enchant_rows.extend(struct.pack('<38I', *entry))
enchants[20 + ec * ez:20 + ec * ez] = enchant_rows
struct.pack_into('<I', enchants, 4, ec + 7)
b[pool_start:pool_start] = child_rows
struct.pack_into('<I', b, 4, n + 8)
(T/'candidate/SpellItemEnchantment.dbc').write_bytes(enchants)
(T/'candidate').mkdir(exist_ok=True);(T/'candidate/Spell.dbc').write_bytes(b)
print('Staged equipment requirements for seven passive spells on the captured Area 52 server catalog')
