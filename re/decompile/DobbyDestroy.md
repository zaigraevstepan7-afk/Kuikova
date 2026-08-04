# DobbyDestroy

- VA: `0x23b680`
- insns: 19

## Field offsets (ldr/str immediates)

```
{
  "16": 2,
  "300": 1
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
    "from": "0x23b688",
    "to": "0x23b6cc"
  },
  {
    "from": "0x23b690",
    "to": "0x23b700"
  },
  {
    "from": "0x23b6a4",
    "to": "0x26cd60"
  },
  {
    "from": "0x23b6a8",
    "to": "0x23b6cc"
  },
  {
    "from": "0x23b6b0",
    "to": "0x23bbd8"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: DobbyDestroy
void DobbyDestroy(...) {
    call(0x0x23b6cc);  // @0x23b688
    call(0x0x23b700);  // @0x23b690
    // branch cbz x0, #0x23b6c0
    call(0x0x26cd60);  // @0x23b6a4
    call(0x0x23b6cc);  // @0x23b6a8
    call(0x0x23bbd8);  // @0x23b6b0
    return;
    return;
}
```
