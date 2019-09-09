open Relude.Globals;
open ReludeParse;

let defaultQuote = "\"";
let defaultEscapes = ["\""];
let defaultDelimiters = [","];
let defaultNewLines = ["\r\n", "\n"];

module Field = {
  let makeParser =
      (
        ~quote=defaultQuote,
        ~escapes=defaultEscapes,
        ~delimiters=defaultDelimiters,
        ~trim=false,
        ~newLines=defaultNewLines,
        (),
      ) => {
    let terminators = List.concat(delimiters, newLines);
    let escapedQuote = Parser.(tries(anyOfStr(escapes) *> str(quote)));
    let quoted =
      Parser.(
        between(
          str(quote),
          str(quote),
          many(escapedQuote <|> anyCharNotIn([quote])),
        )
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

  let parseWithOptions =
      (~escapes=?, ~quote=?, ~delimiters=?, ~trim=?, ~newLines=?, str) => {
    let parser =
      makeParser(~escapes?, ~quote?, ~delimiters?, ~trim?, ~newLines?, ());
    runParser(str, parser);
  };

  let parse = str => parseWithOptions(str);
};

module Record = {
  let makeParser =
      (
        ~size=?,
        ~escapes=?,
        ~quote=?,
        ~delimiters=defaultDelimiters,
        ~trim=?,
        ~newLines=defaultNewLines,
        (),
      ) => {
    let field =
      Field.makeParser(
        ~escapes?,
        ~quote?,
        ~delimiters,
        ~trim?,
        ~newLines,
        (),
      );
    let checkSize = fields =>
      Option.fold(true, Int.eq(List.length(fields)), size);

    Parser.(
      peekNot(eof)
      *> sepBy(anyOfStr(delimiters), field)
      |> filter(checkSize)
      <?> "Expected record size: "
      ++ Option.fold("", Int.toString, size)
    );
  };

  let parseWithOptions =
      (~size=?, ~quote=?, ~delimiters=?, ~trim=?, ~newLines=?, str) => {
    let parser =
      makeParser(~size?, ~quote?, ~delimiters?, ~trim?, ~newLines?, ());
    runParser(str, parser);
  };

  let parse = str => parseWithOptions(str);
};

let makeParser =
    (
      ~escapes=?,
      ~quote=?,
      ~delimiters=?,
      ~trim=?,
      ~newLines=defaultNewLines,
      (),
    ) => {
  let record =
    Record.makeParser(~escapes?, ~quote?, ~delimiters?, ~trim?, ~newLines);

  Parser.(
    record()
    |> flatMap(first => {
         let size = List.length(first);
         let newLines = Parser.anyOfStr(newLines);
         let single = eof *> pure([first]);
         let many =
           newLines
           |> flatMap(_ => sepByOptEnd(newLines, record(~size, ())))
           |> map(rest => [first, ...rest])
           <* eof
           |> catchError(_ => record(~size, ()) |> map(List.pure));

         single <|> many;
       })
  );
};

let defaultParser = makeParser();

let parseWithOptions =
    (~escapes=?, ~quote=?, ~delimiters=?, ~trim=?, ~newLines=?, str) => {
  let parser =
    makeParser(~escapes?, ~quote?, ~delimiters?, ~trim?, ~newLines?, ());
  runParser(str, parser);
};

let parse = Parser.runParser(_, defaultParser);
