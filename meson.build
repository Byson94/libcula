project('libcula', 'c', 
  license: 'MIT',
  default_options: [
    'c_std=c11',
    'warning_level=2',
  ]
)

cc = meson.get_compiler('c')

src_files = run_command('find', 'src', '-name', '*.c', check: true).stdout().strip().split('\n')
sources = files(src_files)

inc_dir = include_directories('include')

libcula = library('cula',
  sources,
  include_directories: inc_dir,
  install: true
)

libcula_dep = declare_dependency(
  include_directories: inc_dir,
  link_with: libcula
)
