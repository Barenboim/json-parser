# JSON parser benchmark (gcc12)

- Date: 2026-11-28 01:36
- CPU: Intel(R) Xeon(R) Platinum 8255C CPU @ 2.50GHz
- C compiler: gcc (GCC) 12.2.1 20221121 (Red Hat 12.2.1-7)
- C++ compiler: g++ (GCC) 12.2.1 20221121 (Red Hat 12.2.1-7)
- Flags: -O3 -march=native -DNDEBUG (all parsers)
- Each sample = parse into a DOM + free it; >= 1.0s and >= 5 samples per cell, after warm-up; one process per cell.
- Throughput uses the best sample; MB = 10^6 bytes.

## Throughput (MB/s, higher is better)

| file | size | json_parser | yyjson | simdjson | sonic-cpp | rapidjson | cJSON | nlohmann |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| nativejson-benchmark/canada.json | 2.3 MB | 321.1 | 634.7 | **691.4** | 684.1 | 191.8 | 68.3 | 56.0 |
| nativejson-benchmark/citm_catalog.json | 1.7 MB | 792.9 | 681.8 | **2265.0** | 1542.1 | 594.3 | 334.3 | 154.8 |
| nativejson-benchmark/twitter.json | 631.5 KB | 519.6 | 767.1 | **2096.2** | 1506.2 | 372.8 | 266.5 | 119.7 |
| json_testsuite/sample.json | 687.5 KB | 603.3 | 591.4 | **2474.4** | 2327.4 | 686.0 | 650.9 | 212.5 |
| jeopardy/jeopardy.json | 55.6 MB | 244.1 | 642.5 | **1426.3** | 541.6 | 275.9 | 162.2 | 73.1 |
| json_nlohmann_tests/all_unicode.json | 13.3 MB | 196.1 | 587.0 | **1320.1** | 554.5 | 442.0 | 147.9 | 100.7 |
| json_nlohmann_tests/all_unicode_ascii.json | 21.9 MB | 279.8 | 807.2 | **1082.6** | 610.6 | 526.6 | 202.5 | 116.1 |
| regression/floats.json | 22.7 MB | 393.5 | 457.2 | **675.0** | 422.7 | 271.0 | 91.6 | 81.8 |
| regression/signed_ints.json | 24.4 MB | 484.2 | 550.7 | **812.8** | 592.0 | 328.7 | 134.2 | 141.3 |
| regression/unsigned_ints.json | 24.4 MB | 483.9 | 559.0 | **793.6** | 464.3 | 331.1 | 135.0 | 136.3 |
| regression/small_signed_ints.json | 12.4 MB | 297.2 | 495.5 | **665.1** | 412.5 | 318.2 | 91.0 | 110.8 |
| small docs (135 files) | 34.6 KB | 376.4 | 935.7 | **1588.4** | 307.2 | 310.4 | 245.9 | 103.8 |

## Speed relative to json_parser (x, higher is faster)

| file | json_parser | yyjson | simdjson | sonic-cpp | rapidjson | cJSON | nlohmann |
|---|---:|---:|---:|---:|---:|---:|---:|
| nativejson-benchmark/canada.json | 1.00 | 1.98 | 2.15 | 2.13 | 0.60 | 0.21 | 0.17 |
| nativejson-benchmark/citm_catalog.json | 1.00 | 0.86 | 2.86 | 1.94 | 0.75 | 0.42 | 0.20 |
| nativejson-benchmark/twitter.json | 1.00 | 1.48 | 4.03 | 2.90 | 0.72 | 0.51 | 0.23 |
| json_testsuite/sample.json | 1.00 | 0.98 | 4.10 | 3.86 | 1.14 | 1.08 | 0.35 |
| jeopardy/jeopardy.json | 1.00 | 2.63 | 5.84 | 2.22 | 1.13 | 0.66 | 0.30 |
| json_nlohmann_tests/all_unicode.json | 1.00 | 2.99 | 6.73 | 2.83 | 2.25 | 0.75 | 0.51 |
| json_nlohmann_tests/all_unicode_ascii.json | 1.00 | 2.89 | 3.87 | 2.18 | 1.88 | 0.72 | 0.42 |
| regression/floats.json | 1.00 | 1.16 | 1.72 | 1.07 | 0.69 | 0.23 | 0.21 |
| regression/signed_ints.json | 1.00 | 1.14 | 1.68 | 1.22 | 0.68 | 0.28 | 0.29 |
| regression/unsigned_ints.json | 1.00 | 1.16 | 1.64 | 0.96 | 0.68 | 0.28 | 0.28 |
| regression/small_signed_ints.json | 1.00 | 1.67 | 2.24 | 1.39 | 1.07 | 0.31 | 0.37 |
| small docs (135 files) | 1.00 | 2.49 | 4.22 | 0.82 | 0.82 | 0.65 | 0.28 |
| **geomean** | **1.00** | **1.63** | **3.07** | **1.76** | **0.94** | **0.45** | **0.29** |

## Median time per parse (ms, lower is better)

| file | json_parser | yyjson | simdjson | sonic-cpp | rapidjson | cJSON | nlohmann |
|---|---:|---:|---:|---:|---:|---:|---:|
| nativejson-benchmark/canada.json | 7.711 | 3.619 | 3.290 | 3.363 | 11.976 | 36.218 | 40.622 |
| nativejson-benchmark/citm_catalog.json | 2.259 | 2.658 | 0.790 | 1.176 | 2.957 | 5.370 | 12.008 |
| nativejson-benchmark/twitter.json | 1.277 | 0.850 | 0.307 | 0.430 | 1.756 | 2.449 | 5.722 |
| json_testsuite/sample.json | 1.176 | 1.186 | 0.284 | 0.301 | 1.016 | 1.075 | 3.288 |
| jeopardy/jeopardy.json | 249.120 | 86.934 | 39.205 | 102.968 | 202.170 | 370.544 | 768.731 |
| json_nlohmann_tests/all_unicode.json | 76.136 | 23.213 | 10.203 | 24.479 | 30.299 | 109.004 | 132.516 |
| json_nlohmann_tests/all_unicode_ascii.json | 89.564 | 27.429 | 20.379 | 36.429 | 41.860 | 138.046 | 199.593 |
| regression/floats.json | 65.298 | 50.346 | 33.760 | 54.282 | 83.952 | 255.597 | 277.131 |
| regression/signed_ints.json | 58.207 | 44.778 | 30.141 | 41.889 | 74.358 | 194.728 | 173.221 |
| regression/unsigned_ints.json | 57.210 | 44.082 | 31.046 | 53.221 | 73.940 | 193.918 | 179.754 |
| regression/small_signed_ints.json | 42.350 | 25.710 | 18.875 | 30.680 | 39.300 | 149.761 | 112.868 |
| small docs (135 files) | 0.094 | 0.038 | 0.022 | 0.114 | 0.113 | 0.143 | 0.352 |

## Verification notes

- json_testsuite/sample.json: string byte counts differ (strings containing \u0000 are cut short by parsers that return NUL-terminated C strings): {'json_parser': 133654, 'yyjson': 134664, 'simdjson': 134664, 'sonic-cpp': 134664, 'rapidjson': 134664, 'cJSON': 133654, 'nlohmann': 134664}
- json_nlohmann_tests/all_unicode.json: string byte counts differ (strings containing \u0000 are cut short by parsers that return NUL-terminated C strings): {'json_parser': 4382591, 'yyjson': 4382592, 'simdjson': 4382592, 'sonic-cpp': 4382592, 'rapidjson': 4382592, 'cJSON': 4382591, 'nlohmann': 4382592}
- json_nlohmann_tests/all_unicode_ascii.json: string byte counts differ (strings containing \u0000 are cut short by parsers that return NUL-terminated C strings): {'json_parser': 4382591, 'yyjson': 4382592, 'simdjson': 4382592, 'sonic-cpp': 4382592, 'rapidjson': 4382592, 'cJSON': 4382591, 'nlohmann': 4382592}
- small docs excluded (not accepted identically by all parsers): spec_tests.json, tests.json, y_object_duplicated_key.json, y_object_duplicated_key_and_value.json
