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
        // also doesn't work?
        // manyUntil(anyOfStr(terminators) |> map(ignore) <|> eof, anyChar)
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
    sepBy(
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
