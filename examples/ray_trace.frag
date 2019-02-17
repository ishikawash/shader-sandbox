#version 120

struct Light
{
        vec3 position;
        vec3 ambientColor;
};

struct Material
{
        vec3 diffuseColor;
        float shininess;
        float reflectance;
};

struct Ray
{
        vec3 origin;
        vec3 direction;
};

struct RayHitInfo
{
        vec3 normal;
        float t;
        int material;
};

uniform float _Time;
uniform vec2 _Mouse;
uniform vec2 _Resolution;

const float INFINITY = 1.0e16;
const float M_PI = 3.14159265;
const float EPSILON = 1.0e-4;
const float GAMMA = 1.0/2.2;

vec3 _BgColor;
Light _Light;
Material _Materials[2];

vec4 QuatFromAxisAngle(vec3 axis, float angle)
{
        float theta = 0.5*angle;
        return vec4(sin(theta)*axis, cos(theta));
}

vec4 QuatConj(vec4 q)
{
        return vec4(-q.xyz, q.w);
}

vec4 QuatMul(vec4 q0, vec4 q1)
{
        float w = q0.w*q1.w - dot(q0.xyz, q1.xyz);
        vec3 v = q1.w*q0.xyz + q0.w*q1.xyz + cross(q1.xyz, q0.xyz);
        return vec4(v, w);
}

vec3 RotateVector(vec3 v, vec3 axis, float angle)
{
        vec4 q = QuatFromAxisAngle(axis, angle);
        vec4 _q = QuatConj(q);
        vec4 _v = QuatMul(QuatMul(q, vec4(v, 0.0)), _q);
        return _v.xyz;
}

bool RayPlaneIntersect(Ray ray, vec3 normal, float d, out RayHitInfo hit)
{
        float a = dot(normal, ray.direction);
        float b = dot(normal, ray.origin);
        float t = (d - b)/a;
        if (t > 0.0) {
                hit.t = t;
                hit.normal = normal;
                return true;
        } else {
                hit.t = INFINITY;
                return false;
        }
}

bool RayCircleIntersect(Ray ray, vec3 normal, vec3 center, float radius, out RayHitInfo hit)
{
        RayHitInfo _hit;
        float d = dot(center, normal);
        if (!RayPlaneIntersect(ray, normal, d, _hit)) {
                hit.t = INFINITY;
                return false;
        }
        vec3 v = (ray.origin + _hit.t*ray.direction) - center;
        float lenSqrt = dot(v, v);
        if (lenSqrt < radius*radius) {
                hit = _hit;
                return true;
        } else {
                hit.t = INFINITY;
                return false;
        }
}

bool RayConeIntersect(Ray ray, vec3 tip, vec3 direction, float angle, float range, out RayHitInfo hit)
{
        vec3 to = ray.origin - tip;
        float k1 = dot(ray.direction, direction);
        float k2 = dot(to, direction);
        float k3 = dot(to, to);
        float k4 = dot(ray.direction, to);
        float cosin = cos(angle);
        float cos2 = cosin*cosin;

        float a = k1*k1 - cos2;
        float b = 2.0*(k1*k2 - k4*cos2);
        float c = k2*k2 - k3*cos2;
        float D = b*b - 4.0*a*c;

        vec2 t = vec2(INFINITY);
        if (D > EPSILON) {
                float rootD = sqrt(D);
                float t1 = (-b + rootD)/(2.0*a);
                float t2 = (-b - rootD)/(2.0*a);
                t[0] = min(t1, t2);
                t[1] = max(t1, t2);
        } else if (abs(D) < EPSILON) {
                t[0] = -b/(2.0*a);
        } else {
                hit.t = INFINITY;
                return false;
        }

        for (int i = 0; i < 2; i++) {
                if (t[i] < 0.0)
                        continue;
                vec3 v1 = (ray.origin + t[i]*ray.direction);
                vec3 v2 = v1 - tip;
                float d = dot(v2, direction);
                if (d > 0.0 && d < range) {
                        vec3 v3 = cross(direction, v2);
                        hit.normal = normalize(cross(v3, v2));
                        hit.t = t[i];
                        return true;
                }
        }
        hit.t = INFINITY;
        return false;
}

bool RayApolloIntersect(Ray ray, vec3 tip, vec3 direction, float angle, float range, out RayHitInfo hit)
{
        RayHitInfo h1;
        if (!RayConeIntersect(ray, tip, direction, angle, range, h1)) {
                hit.t = INFINITY;
                return false;
        }
        vec3 center = tip + range*direction;
        float radius = range*tan(angle);
        RayHitInfo h2;
        if (RayCircleIntersect(ray, direction, center, radius, h2)) {
                if (h1.t < h2.t)
                        hit = h1;
                else
                        hit = h2;
        } else {
                hit = h1;
        }
        return true;
}

bool RaySphereIntersect(Ray ray, vec3 center, float radius, out RayHitInfo hit)
{
        vec3 origin = ray.origin - center;
        float a = dot(ray.direction, ray.direction);
        float b = 2.0 * dot(origin, ray.direction);
        float c = dot(origin, origin) - radius*radius;
        float D = b*b - 4.0*a*c;
        if (D < 0.0) {
                hit.t = INFINITY;
                return false;
        }
        float d = sqrt(D);
        vec2 tt = vec2(-b + d, -b - d) / (2.0*a);
        float t = min(tt.x, tt.y);
        if (t < EPSILON) {
                hit.t = INFINITY;
                return false;
        }
        hit.t = t;
        hit.normal = normalize(origin + t*ray.direction);
        return true;
}

bool RayTrace(Ray ray, out RayHitInfo hit)
{
        RayHitInfo hits[2];
        vec2 offset = 2.0*_Mouse - 1.0;
        offset.y *= -1.0;

        const float ground = -1.0;
        RayPlaneIntersect(ray, vec3(0.0, 1.0, 0.0), ground, hits[1]);
        hits[1].material = 0;

        // vec3 dir = RotateVector(normalize(vec3(0.0, -1.0, 1.0)), vec3(0.0, 1.0, 0.0), 0.5*_Time);
        // float angle = mix(M_PI/12.0, M_PI/6.0, abs(2.0*_Mouse.x - 1.0));
        // float range = mix(1.5, 0.5, _Mouse.y);
        // RayApolloIntersect(ray, vec3(0.0, 0.5, -1.0), dir, angle, range, hits[0]);
        // hits[0].material = 1;

        const float radius = 0.5;
        vec3 center = vec3(sin(_Time * 0.5), ground + radius, cos(_Time) - 2.0);
        RaySphereIntersect(ray, center, radius, hits[0]);
        hits[0].material = 1;

        RayHitInfo h;
        h.t = INFINITY;
        for (int i = 0; i < 2; i++) {
                if (h.t > hits[i].t) {
                        h = hits[i];
                }
        }
        if (h.t < INFINITY) {
                hit = h;
                return  true;
        } else {
                return false;
        }
}

float RayShadow(vec3 point, vec3 direction)
{
        Ray shadowRay;
        shadowRay.origin = point + EPSILON*direction;
        shadowRay.direction = direction;
        RayHitInfo hit;
        if (RayTrace(shadowRay, hit)) {
                return 0.5;
        } else {
                return 0.0;
        }
}

Material GetMaterial(int index)
{
        for (int i = 0; i < 2; i++) {
                if (i == index) {
                        return _Materials[i];
                }
        }
        Material mat;
        return mat;
}

vec3 ApplyLighting(Ray ray, RayHitInfo hit)
{
        float d = dot(hit.normal, -ray.direction);
        if (d < 0.0) {
                return _Light.ambientColor;
        }

        vec3 P = ray.origin + hit.t*ray.direction;
        vec3 V = normalize(-P);
        float D = distance(_Light.position, P);
        vec3 L = (_Light.position - P)/D;

        float shadow = RayShadow(P, L);
        // shadow = 0.0;

        Material mat = GetMaterial(hit.material);

        float kd = clamp(dot(hit.normal, L), 0.0, 1.0);
        kd *= (1.0 - shadow);

        float ks = 0.0;
        if (kd > 0.0 && shadow < EPSILON) {
                vec3 H = normalize(L + V);
                ks = clamp(dot(hit.normal, H), 0.0, 1.0);
                ks = pow(ks, mat.shininess);
        }

        vec3 color = kd*mat.diffuseColor + vec3(ks) + _Light.ambientColor;
        return clamp(color / (0.5*D), 0.0, 1.0);
}

vec4 Reflect(Ray ray, RayHitInfo hit)
{
        vec3 P = ray.origin + hit.t*ray.direction;
        vec3 R = reflect(normalize(P), hit.normal);

        Material mat = GetMaterial(hit.material);

        Ray reflection;
        reflection.origin = P + EPSILON*R;
        reflection.direction = R;
        RayHitInfo _hit;
        if (RayTrace(reflection, _hit)) {
                float cosin = clamp(dot(hit.normal, reflection.direction), 0.0, 1.0);
                float fresnel = mat.reflectance + (1.0 - mat.reflectance)*pow(1.0 - cosin, 5.0);
                vec3 color = clamp(ApplyLighting(reflection, _hit), 0.0, 1.0);
                return vec4(color, fresnel);
        } else {
                return vec4(0.0, 0.0, 0.0, 1.0);
        }
}

vec3 Shade(Ray ray, RayHitInfo hit)
{
        vec3 base = ApplyLighting(ray, hit);
        vec4 reflection = Reflect(ray, hit);
        return mix(reflection.xyz, base, reflection.w);
}

void main()
{
        float aspect = _Resolution.y/_Resolution.x;
        vec2 uv = 2.0*gl_FragCoord.xy/_Resolution - 1.0;
        uv.y = aspect*uv.y;

        _BgColor = vec3(0.0);
        _Light.position = vec3(0.0, 3.0, 0.0);
        _Light.ambientColor = vec3(0.1);

        _Materials[0].diffuseColor = vec3(1.0);
        _Materials[0].shininess = 128.0;
        _Materials[0].reflectance = 0.9;
        _Materials[1].diffuseColor = vec3(0.4);
        _Materials[1].shininess = 128.0;
        _Materials[1].reflectance = 1.0;

        Ray ray;
        ray.origin = vec3(0.0, 0.0, 1.0);
        ray.direction = normalize(vec3(uv, 0.0) - ray.origin);

        RayHitInfo hit;
        vec3 color;
        if (RayTrace(ray, hit)) {
                color = Shade(ray, hit);
        } else {
                color = _BgColor;
        }

        color = pow(color, vec3(GAMMA));

        gl_FragColor = vec4(color, 1.0);
}
