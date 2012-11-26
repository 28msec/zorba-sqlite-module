import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("D:/zorba/code/zorba_modules/sqlite-module/test/Queries/small2.db")
let $isconn := s:is-connected($db)
let $result := s:execute-query($db, "select * from smalltable;")
let $old-db := s:disconnect($db)

return ($result, $isconn)
