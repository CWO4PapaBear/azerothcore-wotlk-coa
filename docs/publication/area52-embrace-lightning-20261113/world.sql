-- The party component excludes the caster. Effect 1 still targets the caster directly.
INSERT INTO conditions
    (SourceTypeOrReferenceId, SourceGroup, SourceEntry, ConditionTypeOrReference,
     ConditionTarget, ConditionValue1, ConditionValue2, NegativeCondition, Comment)
VALUES
    (13, 1, 15290, 33, 0, 1, 0, 1, 'Vampiric Embrace - party heal excludes its caster');
