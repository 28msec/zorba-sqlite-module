import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")
let $isconn := s:is-connected($db)
let $result1 := s:execute-update($db, "CREATE TABLE maintable (id INTEGER primary key asc, name TEXT not null, salary REAL)")
let $result2 := s:execute-update($db, "INSERT INTO maintable (name, salary) values('Luis', 100.0)")
let $result3 := s:execute-update($db, "INSERT INTO maintable (name, salary) values('Juan', 200.0)")
let $result4 := s:execute-update($db, "INSERT INTO maintable (name, salary) values('Rodolfo', 300.0)")
let $result5 := s:execute-query($db, "SELECT id, name, salary FROM maintable")
let $old-db := s:disconnect($db)

return ($isconn, $result1, $result2, $result3, $result4, $result5)