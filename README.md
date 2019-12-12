# Relude CSV

[![GitHub CI](https://img.shields.io/github/workflow/status/reazen/relude-csv/CI/master)](https://github.com/reazen/relude-csv/actions)
[![npm](https://img.shields.io/npm/v/relude-csv.svg)](https://www.npmjs.com/package/relude-csv)
[![Coveralls](https://img.shields.io/coveralls/github/reazen/relude-csv.svg)](https://coveralls.io/github/reazen/relude-csv) 

Parse strings of CSV data into a structured `list(list(string))`, using the parsing tools in [Relude Parse](https://github.com/reazen/relude-parse).

## Features

Relude CSV should be able to parse any [ISO 4180](https://tools.ietf.org/html/rfc4180)-compliant CSV, meaning:

- Fields are separated by commas
- Each record (row) ends with a CRLF newline
- The file may optionally end with a newline, which does not start another row of data
- Fields may or may not be surrounded by double quotes
- Quoted fields may contain commas, newlines, and escaped double quotes
- All records have the same number of fields

In addition to support for the spec, since there's a lot of variation in real-life CSVs, and a lot of non-compliance, the following things are configurable via optional arguments:

| Argument | Default | Spec | Comment |
|----------|---------|------|---------|
| `~newLines` | `["\n", "\r\n"]` | `"\r\n"` | Unix newlines are common enough that this is the one area where our default is more flexible than the spec |
| `~escapes` | `["\""]` | `"\""` | According to the spec, nested double quotes should be escaped with another double quote prefix. We don't support backslash escapes out of the box, but you can easily add that or any other escape character yourself |
| `~quote` | `"\""` | `"\""` | |
| `~delimiters` | `[","]` | `","` | Tab-separated values are another common way to represent tabular data, and you can easily support that with `~delimiters=["\t"]` |
| `~trim` | `false` | `false` | Per the spec, whitespace is considered significant, but this CSV `aaa, bbb, ccc` could really benefit from some trimming. For now, trimming only applies to un-quoted fields |

## Caveats

For now, it's assumed that both the input string and the output list can fit into memory. Streaming data through a parser would be a cool thing to play around with, but so far we haven't had the need. If you want to help, let us know.

But long before you run out of memory, you'll probably run out of stack space because [the underlying parsing is not tail recursive](https://github.com/reazen/relude-parse/issues/21). We're looking into options to fix that, realizing that may limit the usefulness of Relude CSV in the meantime.

## Usage

`ReludeCSV.Parse` includes a `parser` which is a `ReludeParse.Parser.t(list(string))` that you can easily compose with other parsers. More likely, however, you'll just want to parse a string directly:

```reason
ReludeCSV.parse("aaa,bbb\nddd,eee") == Ok([["aaa", "bbb"], ["ccc", "ddd"]]);
```

Additionally, you can build and run a customized parser using the configurations mentioned above:

```reason
ReludeCSV.parse(~trim=true, "aaa, bbb") == Ok(["aaa", "bbb"]);
```

Given an invalid CSV, parsers will fail with a `Belt.Result.Error(ParseError.t)`. Actual error content could be much improved to give you more information about what went wrong:

```reason
ReludeCSV.parse("aaa,bbb\nccc") == Error(ParseError("Expected record size: 2"));
```
