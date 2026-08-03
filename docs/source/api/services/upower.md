# UPower

```{danger}
The UPower service is incomplete.
```

## Data Types

````{doxygenstruct} cula_upower
```{literalinclude} ../../../../include/libcula/services/upower.h
:language: c
:start-at: struct cula_upower {
:end-at: };
:dedent: 0
```
````

````{doxygenstruct} cula_upower_device
```{literalinclude} ../../../../include/libcula/services/upower.h
:language: c
:start-at: struct cula_upower_device {
:end-at: };
:dedent: 0
```
````

````{doxygenstruct} cula_upower_device_property_change_evt
```{literalinclude} ../../../../include/libcula/services/upower.h
:language: c
:start-at: struct cula_upower_device_property_change_evt {
:end-at: };
:dedent: 0
```
````

```{doxygenenum} cula_upower_device_property_type
```

## API

```{doxygenfunction} cula_get_or_create_upower
```

```{doxygenfunction} cula_destroy_upower
```
