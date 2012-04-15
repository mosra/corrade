add_subdirectory(NativePluginAccessor)

# Propagate plugin list variable to parent scope
set(CorradePluginManager_Plugins ${CorradePluginManager_Plugins} PARENT_SCOPE)
