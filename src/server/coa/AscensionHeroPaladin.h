/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#ifndef ASCENSION_HERO_PALADIN_H
#define ASCENSION_HERO_PALADIN_H

class SpellInfo;
class Unit;

void ApplyAscensionHeroPaladinContracts(SpellInfo* spellInfo);

unsigned int Area52FanaticismPeriodicBonus(Unit* caster, Unit* target, SpellInfo const* info);

bool Area52FanaticismMarkAffected(Unit* caster, SpellInfo const* info);

#endif
