#ifndef __RULES_H__
#define __RULES_H__

// Types of superko rules.
enum SuperKo
{
    Situational,
    Positional
};

// Define a set of rules.
struct Rules
{
    double Komi;
    SuperKo Ko;
};

// The AGA ruleset (used in the UK).
Rules AGA = { 7.5, Situational };

Rules CurrentRules = AGA;

#endif // __RULES_H__
