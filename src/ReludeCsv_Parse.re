open Relude.Globals;
open ReludeParse;

let defaultDelimiters = [","];
let defaultNewLines = ["\r\n", "\n"];

module Field = {
  let makeParser =
      (
        ~quote="\"",
        ~delimiters=defaultDelimiters,
        ~trim=false,
        ~newLines=defaultNewLines,
        (),
      ) => {
    let terminators = List.concat(delimiters, newLines);
    let quoted =
      Parser.(
        str(quote)
        *> manyUntil(str(quote), anyChar)
        |> map(List.String.join)
      );

    let unquoted =
      Parser.(
        many(anyCharNotIn(terminators))
        |> map(List.String.join)
        |> map(trim ? String.trim : id)
      );

    // TODO: seems like this should work, but it doesn't
    // let quoted =
    //   Parser.(
    //     between(str(quote), str(quote), many(anyChar))
    //     |> map(List.String.join)
    //   );

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
      sepBy(
        anyOfStr(delimiters),
        Field.makeParser(~quote?, ~delimiters, ~trim?, ~newLines, ()),
      )
    );

  let parseWithOptions = (~delimiters=?, ~trim=?, ~newLines=?, str) =>
    Parser.runParser(str, makeParser(~delimiters?, ~trim?, ~newLines?, ()));

  let parseDefault = str => parseWithOptions(str);
};

let makeParser = (~delimiters=?, ~trim=?, ~newLines=?, ()) =>
  Parser.manyUntil(
    Parser.eof,
    Record.makeParser(~delimiters?, ~trim?, ~newLines?, ()),
  );

let defaultParser = makeParser();

let parse = Parser.runParser(_, defaultParser);
