import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")
let $prep-stmnt1 := s:prepare-statement($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories INTEGER not null)")
let $res1 := s:execute-prepared($prep-stmnt1)
let $prep-stmnt2 := s:prepare-statement($db, "INSERT INTO smalltable (name, calories) VALUES ('carrot', ?)")
let $not := s:set-numeric($prep-stmnt2, 1, "string value")
let $db := s:disconnect($db)

return $prep-stmnt1