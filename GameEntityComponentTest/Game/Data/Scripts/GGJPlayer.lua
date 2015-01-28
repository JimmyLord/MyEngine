-- test script

GGJPlayer =
{

Externs =
{
	-- name, type, initial value
	{ "Speed", "Float", 1 },
};

OnLoad = function()
end,

OnPlay = function()
end,

OnStop = function()
end,

OnTouch = function(action, id, x, y, pressure, size)
	-- LogInfo( "OnTouch " .. id .. " " .. action .. " " .. x .. "\n" );

	this.helda = false;
	this.heldd = false;

	if( action == 2 ) then
		if( x < 320 ) then
			this.helda = true;
		else
			this.heldd = true;
		end
	end
end,

OnButtons = function(action, id)
	--key down
	if( action == 0 ) then
		if( id == 87 or id == 119 ) then -- w
			this.heldw = true;
		end
		if( id == 65 or id == 97 ) then -- a
			this.helda = true;
		end
		if( id == 83 or id == 115 ) then -- s
			this.helds = true;
		end
		if( id == 68 or id == 100 ) then -- d
			this.heldd = true;
		end
	end
	--key up
	if( action == 1 ) then
		if( id == 87 or id == 119 ) then -- w
			this.heldw = false;
		end
		if( id == 65 or id == 97 ) then -- a
			this.helda = false;
		end
		if( id == 83 or id == 115 ) then -- s
			this.helds = false;
		end
		if( id == 68 or id == 100 ) then -- d
			this.heldd = false;
		end
	end
end,

Tick = function(timepassed)
	local force = Vector3( 0,0,0 );

	if( this.heldw == true ) then
		force = force:Add( Vector3(0,0,-1) );
	end
	if( this.helda == true ) then
		force = force:Add( Vector3(-1,0,0) );
	end
	if( this.helds == true ) then
		force = force:Add( Vector3(0,0,1) );
	end
	if( this.heldd == true ) then
		force = force:Add( Vector3(1,0,0) );
	end

	if( force:LengthSquared() ~= 0 ) then
		local transform = this.gameobject:GetTransform();
		local pos = transform:GetPosition();

		pos = pos:Add( force:Scale( timepassed * this.Speed ) );
		transform:SetPosition( pos );
	end

end,

}