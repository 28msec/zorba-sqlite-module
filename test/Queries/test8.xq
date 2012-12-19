import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")

return {
  variable $isconn := s:is-connected($db);
  variable $result1 := s:execute($db, "CREATE TABLE maintable (id INTEGER primary key asc, name TEXT not null, salary REAL)");
  variable $result2 := s:execute($db, "INSERT INTO maintable (name, salary) values('Luis', 100.0)");
  variable $result3 := s:execute($db, "INSERT INTO maintable (name, salary) values('Juan', 200.0)");
  variable $result4 := s:execute($db, "INSERT INTO maintable (name, salary) values('Rodolfo', 300.0)");
  variable $result5 := s:execute($db, "SELECT id, name, salary FROM maintable");
  variable $old-db := s:disconnect($db);

  ($isconn, $result1, $result2, $result3, $result4, $result5)
}