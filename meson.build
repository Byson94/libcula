project('libcula', 'c', 
  license: 'MIT',
  default_options: [
    'c_std=gnu11',
    'warning_level=3',
    'default_library=both'
  ]
)

cc = meson.get_compiler('c')
add_project_arguments('-D_GNU_SOURCE', language: 'c')

# Dependency
wayland_client = dependency('wayland-client')
systemd_dep = dependency('libsystemd')
libuv_dep = dependency('libuv')
cjson_dep = dependency('libcjson')
neco_proj = subproject('neco')
neco_dep = neco_proj.get_variable('neco_dep')

# Source
src_files = run_command('find', 'src', '-name', '*.c', check: true).stdout().strip().split('\n')
sources = files(src_files)

inc_dir = include_directories('include')

libcula = library('cula',
  sources,
  include_directories: inc_dir,
  dependencies: [
    wayland_client,
    systemd_dep,
    libuv_dep,
    cjson_dep,
    neco_dep
  ],
  install: true
)

subdir('tests')
