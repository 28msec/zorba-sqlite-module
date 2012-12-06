import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")
let $results := s:execute-query($db, "SELECT id, name, calories FROM smalltable")
let $db := s:disconnect($db)

return $results