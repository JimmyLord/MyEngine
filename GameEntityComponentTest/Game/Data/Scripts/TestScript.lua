
function OnPlay()
	--LogInfo( "OnPlay Start\n" )

	transform = this:GetTransform()
	transform:SetPositionXYZ( 350, 650, 0 )

	--LogInfo( "OnPlay End" .. transform:GetPositionX() .. "\n" )
end

function OnStop()
	--LogInfo( "OnStop\n" )
end

function Tick(timepassed)
	--LogInfo( "Tick Start\n" )

	transform = this:GetTransform()
	x = transform:GetPositionX()
	
	speed = 100
	transform:SetPositionXYZ( x + timepassed*speed, 650, 0 )
	
	--LogInfo( "Tick End" .. transform:GetPositionX() .. "\n" )
end
