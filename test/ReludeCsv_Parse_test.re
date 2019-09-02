open Jest;
open Expect;
open Relude.Globals;

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
});

describe("Parse record", () => {
  module Record = ReludeCsv.Parse.Record;

  test("one record, simple fields", () =>
    expect(Record.parseDefault("aaa,bbb"))
    |> toEqual(Result.ok(["aaa", "bbb"]))
  );

  test("stops at newline", () =>
    expect(Record.parseDefault("aaa,bbb\nccc,ddd"))
    |> toEqual(Result.ok(["aaa", "bbb"]))
  );

  test("custom delimiter", () =>
    expect(Record.parseWithOptions(~delimiters=[";"], "aaa;bbb"))
    |> toEqual(Result.ok(["aaa", "bbb"]))
  );
});

Skip.describe("Parse CSV", () => {
  let parse = ReludeCsv.Parse.parse;

  // TODO: maybe at least one record is required?
  Skip.test("empty", () =>
    expect(parse("")) |> toEqual(Result.ok([[]]))
  );

  test("Simple fields, two records", () =>
    expect(
      parse("aaa,bbb\nccc,ddd")
      |> Result.map(List.map(List.toArray) >> List.toArray),
    )
    |> toEqual(Result.ok([|[|"aaa", "bbb"|], [|"ccc", "ddd"|]|]))
  );
});
