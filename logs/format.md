# Log Format Description

## Logs are output by g3log in the form:
LOG format: [YYYY/MM/DD hh:mm:ss uuu* LEVEL FILE->FUNCTION:LINE] messagen
(uuu*: microseconds fractions of the seconds value)

## Example Output
A function entry would look like
```
2020/05/23 16:23:21 110646 INFO [experimenter.cc->experiment_fentry:146] 30936: UpdateStyleAndLayoutTree 11110000
    $1        $2      $3    $4             $5                             $6              $7                $8
```
A function exit would look like

```
2020/05/23 16:23:21 168378 INFO [experimenter.cc->experiment_fexit:158] 30936: UpdateStyleAndLayoutTree 11111111 21.6936
    $1        $2      $3    $4                    $5                      $6               $7               $8      $9
```


## AWK field numbers
|Field # |     Value   |
|:-------|------------:|
|   $1   | Date        |
|   $2   | Time        |
|   $3   | Micros      |
|   $4   | LOGLVL      |
|   $5   | File        |
|   $6   | TID         |
|   $7   | Function    |
|   $8   | CPU Mask    |
|   $9   | Latency(ms) |
