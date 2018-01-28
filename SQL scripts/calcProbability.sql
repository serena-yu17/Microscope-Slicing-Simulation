drop function calcProbability();

create or replace function calcProbability() 
returns table(
	sample numeric(17, 1),
	probability numeric(14, 5)
) as
$$
declare
	rowVal record;
    pSL double precision;
    totalProb double precision;
    countRadiusQ bigint;
    countRadiusS bigint;
begin
select sum(sample.probability) into totalProb
    from sample;    
pSL := 1;
for rowVal in (select * from sliceCountAll())
loop
    pSL := pSL * combination(rowVal.countAll, rowVal.countMatch);
end loop;

select count(query.radius) into countRadiusQ from query;
select count(slice.slice) into countRadiusS from slice;
pSL := pSL / combination(countRadiusS, countRadiusQ);

return QUERY 
    select cast(cast(s.sample as numeric (17, 1))/10 as numeric (17, 1)), cast(bayes(s.sample, pSL, totalProb) * 100 as numeric(14, 5))
    from query q, slice s
    where q.radius = round(s.slice/10)
    group by s.sample
    having count(distinct q.radius) >= (select count(distinct radius) from query)
    order by bayes(s.sample, pSL, totalProb) desc;
end
$$
language plpgsql;