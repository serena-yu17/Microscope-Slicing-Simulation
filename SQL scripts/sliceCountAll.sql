create or replace function sliceCountAll()
returns table(
    radius int, countMatch bigint, countAll bigint
) as
$$
	select a.radius, countMatch, countAll  
from
    (select q.radius, count(q.radius) as countMatch
    from query q
    group by q.radius) as m,
    (select q.radius, count(s.slice) as countAll
    from query q, slice s
    where round(s.slice/10) = round(q.radius)
    group by q.radius) as a
where a.radius = m.radius;
$$ language sql;
