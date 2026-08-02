project('neco', 'c', version: '1.0.0')

neco_lib = static_library('neco', 'neco.c', c_args: '-std=c17')
neco_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with: neco_lib
)

