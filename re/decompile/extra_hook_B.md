# extra_hook_B

- VA: `0x1d83cc`
- insns: 16

## Field offsets (ldr/str immediates)

```
{
  "16": 2,
  "1552": 1,
  "1560": 1,
  "32": 1,
  "1441": 1
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
// recovered pseudocode: extra_hook_B
void extra_hook_B(...) {
    // branch cbz w8, #0x1d83f8
    return;
}
```
