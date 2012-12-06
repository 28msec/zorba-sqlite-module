import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("non-existent-file.db", {"open-read-only" : false, "open-create" : false})
let $results := s:execute-query($db, "SELECT id, name, calories FROM smalltable")
let $db := s:disconnect($db)

return $results