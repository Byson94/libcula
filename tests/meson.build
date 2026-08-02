test_upower = executable('test_upower',
  sources: ['test_upower.c'],
  link_with: libcula,
  include_directories: include_directories('../include')
)
test_dbus = executable('test_dbus',
  sources: ['test_dbus.c'],
  link_with: libcula,
  include_directories: include_directories('../include/')
)
test_dbus_advanced = executable('test_dbus_advanced',
  sources: ['test_dbus_advanced.c'],
  link_with: libcula,
  include_directories: include_directories('../include/')
)

test('upower integration test', test_upower)
test('dbus integration test', test_dbus)
test('advaced dbus integration test', test_dbus_advanced)

