from pathlib import Path
root=Path(__file__).resolve().parents[3]
s=(root/'src/server/coa/Area52MysticScrolls.inc').read_text();registration=(root/'src/server/coa/AscensionMysticEnchant.cpp').read_text()
a=s.index('    if (packet.GetOpcode() == 0x611)');b=s.index('    if (packet.GetOpcode() == 0x607)',a);purchase=s[a:b]
assert 'Claim(0x611, QueueScrollRequest)' in registration
for guard in ('packet.size()', '!scroll', 'GetMoney() < scroll->BuyPrice', 'CanStoreNewItem', '!NearAltar(player)', '!CanChange(player, state)'):
    assert guard in purchase
assert purchase.index('StoreNewItem(destination')<purchase.index('ModifyMoney(')
assert 'GetItemTemplate(992720)' in purchase
assert 'SaveInventoryAndGoldToDB(transaction)' in purchase
assert 'SendResult(player, 0x612, result)' in purchase
for result in ('RE_PURCHASE_OK','RE_PURCHASE_UNKNOWN','RE_PURCHASE_ITEM_NOT_FOUND','RE_PURCHASE_NOT_ENOUGH_MONEY','RE_PURCHASE_NOT_ENOUGH_SPACE','RE_PURCHASE_NO_MYSTIC_ALTAR','RE_PURCHASE_NOT_WHILE_CASTING'):
    assert result in purchase
print('PASS: native request/response binding, guards, charge-after-store and persistence path; runtime client click remains acceptance testing.')
