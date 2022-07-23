vec2 p1 = vec2(500.0,70.0);
vec2 p2 = vec2(400.0,300.0);


//distance between a point and an infinite line
float lineDist(vec2 pt1, vec2 pt2, vec2 pt3)
{
   vec2 v1 = pt2 - pt1;
   vec2 v2 = pt1 - pt3;
   vec2 v3 = vec2(v1.y,-v1.x);
   return abs(dot(v2,normalize(v3)));
}

//distance between a point and a line segment
float segDist(vec2 p1, vec2 p2, vec2 p)
{
    float param = dot((p - p1),(p2-p1))/pow(length(p2-p1),2.0);
    
    if(param < 0.0f){ // p is closer to p1
        return length(p1-p);
    }else if(param > 1.0f){ // p is closer to p2
        return length(p2-p);
    }
    // closer to a point inbetween p1 and p2
    return length((p1 + param*(p2-p1))-p); 
}

float maxRange(vec2 p1, vec2 p2, vec2 res){
    return max(
        max(
            max(res.x - p2.x,res.x - p1.x),  //dist p1 and p2 to right 
            max(p2.x,p1.x) //dist p1 and p2 to left 
            ),
        max(
            max(res.y - p2.y,res.y - p1.y),  //dist p1 and p2 to top 
            max(p2.y,p1.y) //dist p1 and p2 to bottom 
            )
        );
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    
    float dist = segDist(p1,p2,fragCoord);
    
    float value = dist/(maxRange(p1,p2,iResolution.xy));
    
    
    // Time varying pixel color
    vec3 col = vec3(value,value,value);
    if(value > 0.9999f){
        col = vec3(0.0f,1.0f,1.0f);
    }

    // Output to screen
    fragColor = vec4(col,1.0f);
}
