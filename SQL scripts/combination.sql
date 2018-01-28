create OR REPLACE function combination(nv bigint, mv bigint) returns double precision as
$$
declare 
	res double precision;
    i int;
begin
	if (nv < mv) then
    	return 0;
    end if;
	res := 1;
    i := nv;
    while (i > nv-mv) loop
    	res := res * i;
        i := i-1;
    end loop;
    res := res / factorial(mv);
    return res;
end
$$ language plpgsql;