CREATE OR REPLACE FUNCTION get_order_data(receive_date text)
	RETURNS text AS $$
DECLARE 
	order_data TEXT DEFAULT '';
	rec_order   RECORD;
	cur_orders CURSOR(receive_date text) 
	FOR SELECT 
	FROM ric_ord;
BEGIN
	-- Open the cursor
	OPEN cur_orders(receive_date);

	LOOP
		-- fetch row into the record
		FETCH cur_cur_orders INTO rec_order;
		-- exit when no more row to fetch
		EXIT WHEN NOT FOUND;

		-- build the output
		order_data := rec_order.ordprog || ',' || rec_order.ronmcll ;
	END LOOP;

	-- Close the cursor
	CLOSE cur_orders;

	RETURN order_data;
END; $$

LANGUAGE plpgsql;

