
set(COMPONENT_SRCS "demesh.c")

set(COMPONENT_ADD_INCLUDEDIRS "")

# requirements can't depend on config
set(COMPONENT_REQUIRES mcommon mconfig mwifi mlink)

register_component()
