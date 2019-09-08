open Jest;
open Expect;
open Relude.Globals;

let parseError = str => ReludeParse.Parser.ParseError.ParseError(str);

describe("playground", () => {
  open ReludeParse;
  let newlineEof = Parser.(str("\n") *> eof);
  let maybeChar =
    Parser.(many(anyCharNotIn(["\n"])) |> map(List.String.join));
  let withoutLastNewline =
    Parser.(sepByOptEnd(str("\n"), peekNot(eof) *> maybeChar));

  test("should match?", () =>
    expect(runParser("\n", newlineEof)) |> toEqual(Result.ok())
  );

  test("discard final newline", () =>
    expect(runParser("a\nb\nc\n", withoutLastNewline))
    |> toEqual(Result.ok(["a", "b", "c"]))
  );
});

describe("Parse field", () => {
  module Field = ReludeCsv.Parse.Field;

  test("one field", () =>
    expect(Field.parse("aaa")) |> toEqual(Result.ok("aaa"))
  );

  test("empty string", () =>
    expect(Field.parse("")) |> toEqual(Result.ok(""))
  );

  test("stop at comma", () =>
    expect(Field.parse("aaa,bbb")) |> toEqual(Result.ok("aaa"))
  );

  test("stop at newline", () =>
    expect(Field.parse("aaa\nbbb")) |> toEqual(Result.ok("aaa"))
  );

  test("stop at custom delimiter", () =>
    expect(Field.parseWithOptions(~delimiters=[";"], "aaa,bbb;ccc"))
    |> toEqual(Result.ok("aaa,bbb"))
  );

  test("fields may be quoted", () =>
    expect(Field.parse("\"aaa\"")) |> toEqual(Result.ok("aaa"))
  );

  test("quoted can contain delimiter", () =>
    expect(Field.parse("\"aa,aa\",bbb")) |> toEqual(Result.ok("aa,aa"))
  );

  test("quoted can contain custom delimiter", () =>
    expect(Field.parseWithOptions(~delimiters=["\t"], "\"a\ta\""))
    |> toEqual(Result.ok("a\ta"))
  );

  test("quoted can contain newlines", () =>
    expect(Field.parse("\"aa\naa\",bb")) |> toEqual(Result.ok("aa\naa"))
  );

  test("whitespace is significant (by default)", () =>
    expect(Field.parse(" aaa  , bbb ")) |> toEqual(Result.ok(" aaa  "))
  );

  test("whitespace can be trimmed (optionally)", () =>
    expect(Field.parseWithOptions(~trim=true, "\t\taaa  ,"))
    |> toEqual(Result.ok("aaa"))
  );

  test("Quotes must be matched", () =>
    expect(Field.parse("\"aaa,bbb") |> Result.isOk) |> toEqual(false)
  );

  test("Quote value can be changed", () =>
    expect(Field.parseWithOptions(~quote="\'", "\'aaa\nbbb\'"))
    |> toEqual(Result.ok("aaa\nbbb"))
  );

  test("Regular quotes aren't ignored when quote is changed", () =>
    expect(Field.parseWithOptions(~quote="\'", "\"aaa\""))
    |> toEqual(Result.ok("\"aaa\""))
  );

  test("parse complex newline", () =>
    expect(Field.parse("aaa\r\nbbb")) |> toEqual(Result.ok("aaa"))
  );
});

describe("Parse record", () => {
  module Record = ReludeCsv.Parse.Record;

  test("one record, simple fields", () =>
    expect(Record.parse("aaa,bbb")) |> toEqual(Result.ok(["aaa", "bbb"]))
  );

  test("stops at newline", () =>
    expect(Record.parse("aaa,bbb\nccc,ddd"))
    |> toEqual(Result.ok(["aaa", "bbb"]))
  );

  test("custom delimiter", () =>
    expect(Record.parseWithOptions(~delimiters=[";"], "aaa;bbb"))
    |> toEqual(Result.ok(["aaa", "bbb"]))
  );

  test("allows fields to have quoted newlines", () =>
    expect(Record.parse("\"aaa\nbbb\",ccc\nddd,eee"))
    |> toEqual(Result.ok(["aaa\nbbb", "ccc"]))
  );

  test("parse complex newline", () =>
    expect(Record.parse("aaa,bbb\r\nyyy,zzz") |> Result.map(List.toArray))
    |> toEqual(Result.ok([|"aaa", "bbb"|]))
  );

  test("required number of fields (fail: too few)", () =>
    expect(Record.parseWithOptions(~size=3, "aaa,bbb"))
    |> toEqual(Result.error(parseError("Expected record size: 3")))
  );

  test("required number of fields (fail: too many)", () =>
    expect(Record.parseWithOptions(~size=1, "aaa,bbb"))
    |> toEqual(Result.error(parseError("Expected record size: 1")))
  );

  test("required number of fields (success)", () =>
    expect(Record.parseWithOptions(~size=1, "aaa"))
    |> toEqual(Result.ok(["aaa"]))
  );
});

describe("Parse CSV", () => {
  let parse = ReludeCsv.Parse.parse;

  // TODO: i have absolutely no idea what we should do here...
  // is this a CSV with zero rows? one row with zero cells?
  Skip.test("empty", () =>
    expect(parse("")) |> toEqual(Result.ok([[]]))
  );

  test("simple fields, one record", () =>
    expect(parse("aaa,bbb")) |> toEqual(Result.ok([["aaa", "bbb"]]))
  );

  test("simple fields, two records", () =>
    expect(parse("aaa,bbb\nccc,ddd"))
    |> toEqual(Result.ok([["aaa", "bbb"], ["ccc", "ddd"]]))
  );

  test("multi-char newline", () =>
    expect(parse("aaa,bbb\r\nyyy,zzz"))
    |> toEqual(Result.ok([["aaa", "bbb"], ["yyy", "zzz"]]))
  );

  test("trailing newline discarded", () =>
    expect(parse("aaa\n")) |> toEqual(Result.ok([["aaa"]]))
  );

  test("final newline discarded (multiple newlines)", () =>
    expect(parse("aaa\n\n\n"))
    |> toEqual(Result.ok([["aaa"], [""], [""]]))
  );

  test("size of second row is greater than first", () =>
    expect(parse("aaa,bbb\r\nccc,ddd,eee"))
    |> toEqual(Result.error(parseError("Expected record size: 2")))
  );

  test("size of second row is less than first", () =>
    expect(parse("aaa,bbb\r\naaa\r\nccc,ddd\r\nddd,eee,fff,ggg"))
    |> toEqual(Result.error(parseError("Expected record size: 2")))
  );
});
