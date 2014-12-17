
LogInfo( "Start\n" )

transform = this:GetTransform()
transform:SetPositionXYZ( 350, 350, 0 )

LogInfo( "End" .. transform:GetPositionX() .. "\n" )
