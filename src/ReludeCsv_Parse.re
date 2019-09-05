open Relude.Globals;
open ReludeParse;

let defaultQuote = "\"";
let defaultDelimiters = [","];
let defaultNewLines = ["\r\n", "\n"];

module Field = {
  let makeParser =
      (
        ~quote=defaultQuote,
        ~delimiters=defaultDelimiters,
        ~trim=false,
        ~newLines=defaultNewLines,
        (),
      ) => {
    let terminators = List.concat(delimiters, newLines);
    let quoted =
      Parser.(
        between(str(quote), str(quote), many(anyCharNotIn([quote])))
        |> map(List.String.join)
      );

    let unquoted =
      Parser.(
        manyUntilPeek(anyOfStr(terminators) |> orEOF, anyChar)
        |> map(List.String.join)
        |> map(trim ? String.trim : id)
      );

    Parser.(quoted <|> unquoted);
  };

  let parseWithOptions = (~quote=?, ~delimiters=?, ~trim=?, ~newLines=?, str) =>
    Parser.runParser(
      str,
      makeParser(~quote?, ~delimiters?, ~trim?, ~newLines?, ()),
    );

  let parse = str => parseWithOptions(str);
};

module Record = {
  let makeParser =
      (
        ~quote=?,
        ~delimiters=defaultDelimiters,
        ~trim=?,
        ~newLines=defaultNewLines,
        (),
      ) =>
    Parser.(
      peekNot(eof)
      *> sepBy(
           anyOfStr(delimiters),
           Field.makeParser(~quote?, ~delimiters, ~trim?, ~newLines, ()),
         )
    );

  let parseWithOptions = (~quote=?, ~delimiters=?, ~trim=?, ~newLines=?, str) =>
    Parser.runParser(
      str,
      makeParser(~quote?, ~delimiters?, ~trim?, ~newLines?, ()),
    );

  let parse = str => parseWithOptions(str);
};

let makeParser =
    (~quote=?, ~delimiters=?, ~trim=?, ~newLines=defaultNewLines, ()) =>
  Parser.(
    sepByOptEnd(
      anyOfStr(newLines),
      Record.makeParser(~quote?, ~delimiters?, ~trim?, ~newLines, ()),
    )
  );

let defaultParser = makeParser();

let parseWithOptions = (~quote=?, ~delimiters=?, ~trim=?, ~newLines=?, str) =>
  Parser.runParser(
    str,
    makeParser(~quote?, ~delimiters?, ~trim?, ~newLines?, ()),
  );

let parse = Parser.runParser(_, defaultParser);
