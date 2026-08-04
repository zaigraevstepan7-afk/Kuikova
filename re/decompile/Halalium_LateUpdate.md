# Halalium_LateUpdate

- VA: `0x1d7ec4`
- insns: 36

## Field offsets (ldr/str immediates)

```
{
  "48": 2,
  "64": 2,
  "80": 2,
  "96": 2,
  "112": 2,
  "128": 2,
  "144": 2,
  "40": 2,
  "1520": 1,
  "1272": 1,
  "8": 1
}
```

## Strings referenced

```
[]
```

## BL targets

```
[]
```

## Rough pseudocode

```c
// recovered pseudocode: Halalium_LateUpdate
void Halalium_LateUpdate(...) {
    // branch cbz x8, #0x1d7f0c
    // branch b.eq #0x1d7f54
    // branch b.ne #0x1d81b8
    return;
}
```
