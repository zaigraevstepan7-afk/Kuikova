# JNI_OnLoad

- VA: `0x1d69e4`
- insns: 70

## Field offsets (ldr/str immediates)

```
{
  "40": 4,
  "32": 2,
  "48": 2,
  "64": 2,
  "16": 2
}
```

## Strings referenced

```
[]
```

## BL targets

```
[
  {
    "from": "0x1d6a08",
    "to": "0x26c330"
  },
  {
    "from": "0x1d6a10",
    "to": "0x26c340"
  },
  {
    "from": "0x1d6a18",
    "to": "0x26c330"
  },
  {
    "from": "0x1d6a38",
    "to": "0x26c350"
  },
  {
    "from": "0x1d6a48",
    "to": "0x1d6d04"
  },
  {
    "from": "0x1d6a50",
    "to": "0x26c360"
  },
  {
    "from": "0x1d6a58",
    "to": "0x26c370"
  },
  {
    "from": "0x1d6aa0",
    "to": "0x26c380"
  },
  {
    "from": "0x1d6aac",
    "to": "0x26c370"
  },
  {
    "from": "0x1d6abc",
    "to": "0x26c390"
  },
  {
    "from": "0x1d6acc",
    "to": "0x26c3a0"
  },
  {
    "from": "0x1d6adc",
    "to": "0x1d6d04"
  },
  {
    "from": "0x1d6af4",
    "to": "0x26841c"
  },
  {
    "from": "0x1d6af8",
    "to": "0x26c3b0"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: JNI_OnLoad
void JNI_OnLoad(...) {
    call(0x0x26c330);  // @0x1d6a08
    call(0x0x26c340);  // @0x1d6a10
    call(0x0x26c330);  // @0x1d6a18
    call(0x0x26c350);  // @0x1d6a38
    // branch cbnz w0, #0x1d6a88
    call(0x0x1d6d04);  // @0x1d6a48
    call(0x0x26c360);  // @0x1d6a50
    call(0x0x26c370);  // @0x1d6a58
    // branch b.ne #0x1d6af8
    return;
    // branch b.ne #0x1d6af8
    call(0x0x26c380);  // @0x1d6aa0
    call(0x0x26c370);  // @0x1d6aac
    // branch b #0x1d6ae0
    call(0x0x26c390);  // @0x1d6abc
    // branch b #0x1d6ac8
    call(0x0x26c3a0);  // @0x1d6acc
    // branch b #0x1d6ae0
    call(0x0x1d6d04);  // @0x1d6adc
    // branch b.ne #0x1d6af8
    call(0x0x26841c);  // @0x1d6af4
    call(0x0x26c3b0);  // @0x1d6af8
}
```
