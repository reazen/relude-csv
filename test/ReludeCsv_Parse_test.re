open Jest;
open Expect;
open Relude.Globals;

describe("Parser playground", () => {
  open ReludeParse;
  let run = Parser.runParser;

  let endOfField =
    Parser.(anyOfStr([",", "\r\n", "\n"]) |> map(ignore) <|> eof);

  let unquoted =
    Parser.(manyUntil(endOfField, anyChar) |> map(List.String.join));

  let record = Parser.(sepBy(str(","), unquoted));

  test("unquoted, single field", () =>
    expect(run("aaa", unquoted)) |> toEqual(Result.ok("aaa"))
  );

  test("unquoted, first of two fields", () =>
    expect(run("aaa,bbb", unquoted)) |> toEqual(Result.ok("aaa"))
  );

  test("unquoted, two separated by newline", () =>
    expect(run("aaa\nbbb", unquoted)) |> toEqual(Result.ok("aaa"))
  );

  test("unquoted, two separated by crlf newline", () =>
    expect(run("aaa\r\nbbb", unquoted)) |> toEqual(Result.ok("aaa"))
  );

  test("record, one field", () =>
    expect(run("aaa", record)) |> toEqual(Result.ok(["aaa"]))
  );

  test("record, two fields", () =>
    expect(run("aaa,bbb", record)) |> toEqual(Result.ok(["aaa", "bbb"]))
  );

  test("record, two separated by newline", () =>
    expect(run("aaa\r\nbbb", record)) |> toEqual(Result.ok(["aaa"]))
  );
});

Skip.describe("Parse field", () => {
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

Skip.describe("Parse record", () => {
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
});

Skip.describe("Parse CSV", () => {
  let parse = ReludeCsv.Parse.parse;

  // TODO: maybe at least one record is required?
  Skip.test("empty", () =>
    expect(parse("")) |> toEqual(Result.ok([[]]))
  );

  test("simple fields, two records", () =>
    expect(parse("aaa,bbb\nccc,ddd"))
    |> toEqual(Result.ok([["aaa", "bbb"], ["ccc", "ddd"]]))
  );

  test("trailing newline is allowed", () =>
    expect(parse("aaa,bbb\r\nyyy,zzz"))
    |> toEqual(Result.ok([["aaa", "bbb"], ["yyy", "zzz"]]))
  );
});
