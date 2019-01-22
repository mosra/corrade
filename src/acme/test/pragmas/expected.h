int a;
float b;

void foobar();

// Only this gets to the output, in fact.
/** @brief Docs */
DebugStuff apiNames(int a = "disambiguate"[0]);

/* Forced defines also make it possible to get rid of include guards, so this
   comment is not wrapped in any */
