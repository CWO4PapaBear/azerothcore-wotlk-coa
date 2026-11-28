#include "Area52BuildProtocol.h"
#include <iostream>
int main() {
 Area52Build::Entry e; e.Id="900001"; e.Author="Fixture Author"; e.Name="Fixture Crusader"; e.Class=10; e.Category=8;
 e.Description="Synthetic offline build"; e.Icon="Spell_Holy_SealOfMight";
 for(auto pair : {std::pair<unsigned,unsigned>{64127,46},{64129,45},{31879,48},{31880,49},{31881,47}}) { Area52Build::Spell s; s.Id=pair.first; s.Level=pair.second; e.Spells.push_back(s); }
 constexpr char digits[]="0123456789abcdef"; for(auto b:Area52Build::Encode(e)) std::cout<<digits[b>>4]<<digits[b&15]; std::cout<<"\n";
}
