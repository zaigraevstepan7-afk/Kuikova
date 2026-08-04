# DobbySymbolResolver

- VA: `0x23c5d8`
- insns: 16

## Field offsets (ldr/str immediates)

```
{
  "16": 3,
  "32": 2
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
    "from": "0x23c5ec",
    "to": "0x26ccf0"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: DobbySymbolResolver
void DobbySymbolResolver(...) {
    call(0x0x26ccf0);  // @0x23c5ec
    // branch cbz x0, #0x23c600
    return;
    // branch b #0x23bfa8
}
```
