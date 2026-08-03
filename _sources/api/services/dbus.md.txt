# DBus

## Data Types

````{doxygenstruct} cula_dbus
```{literalinclude} ../../../../include/libcula/services/dbus.h
:language: c
:start-at: struct cula_dbus {
:end-at: };
:dedent: 0
```
````

````{doxygenstruct} cula_dbus_call_ctx
```{literalinclude} ../../../../include/libcula/services/dbus.h
:language: c
:start-at: struct cula_dbus_call_ctx {
:end-at: };
:dedent: 0
```
````

````{doxygenstruct} cula_dbus_call_result
```{literalinclude} ../../../../include/libcula/services/dbus.h
:language: c
:start-at: struct cula_dbus_call_result {
:end-at: };
:dedent: 0
```
````

```{doxygenenum} cula_dbus_type
```

```{doxygenenum} cula_dbus_status
```

```{doxygenenum} cula_dbus_call_ctx_type
```

## API

```{doxygenfunction} cula_get_or_create_dbus
```

```{doxygenfunction} cula_create_dbus_call
```

```{doxygenfunction} cula_call_dbus
```

```{doxygenfunction} cula_destroy_dbus_call
```

```{doxygenfunction} cula_destroy_dbus
```

