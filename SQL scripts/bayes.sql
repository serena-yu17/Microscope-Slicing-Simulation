create or replace function bayes(sampleDiam int, pSL double precision, totalProb double precision) returns double precision as
$$
declare
   matchSampleProb real;
   rowVal record;
   sliceCount int;
   res double precision;
   countRadius bigint;
   countRadiusSample bigint;
begin   
    select sum(probability) into matchSampleProb
    from sample
    where sample = sampleDiam;
    res := matchSampleProb / totalProb;
    
    for rowVal in (select * from sliceCountOnSample(sampleDiam)) 
    loop
  		res := res * combination(rowVal.countAll, rowVal.countMatch); 
    end loop;
    
    select count(query.radius) into countRadius from query;
    select count(slice.slice) into countRadiusSample from slice where sample = sampleDiam;
    res := res / combination(countRadiusSample, countRadius);
    
   	res := res / pSL;
    return res;
end
$$
language plpgsql;