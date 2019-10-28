var exports = module.exports;
var gles2 = require('../build/Release/gles2');
// Main object.
function WebGLRenderingContext() {
    this.gl = new gles2.WebGLRenderingContext();
}

// Support objects.
function WebGLProgram(_) { this._ = _; };
function WebGLShader(_) { this._ = _; };
function WebGLBuffer(_) { this._ = _; };
function WebGLFramebuffer(_) { this._ = _; };
function WebGLRenderbuffer(_) { this._ = _; };
function WebGLTexture(_) { this._ = _; };
function WebGLActiveInfo(_) { this._=_; this.size=_.size; this.type=_.type; this.name=_.name; };
function WebGLUniformLocation(_) { this._ = _; };

exports.WebGLRenderingContext = WebGLRenderingContext;
exports.WebGLProgram = WebGLProgram;
exports.WebGLShader = WebGLShader;
exports.WebGLBuffer = WebGLBuffer;
exports.WebGLFramebuffer = WebGLFramebuffer;
exports.WebGLRenderbuffer = WebGLRenderbuffer;
exports.WebGLTexture = WebGLTexture;
exports.WebGLActiveInfo = WebGLActiveInfo;
exports.WebGLUniformLocation = WebGLUniformLocation;

// The singleton webgl render context.
exports.instance = new WebGLRenderingContext();

// The following constants were extracted from the Broadcom's gl2.h file.

/* OpenGL ES core versions */
WebGLRenderingContext.prototype.ES_VERSION_2_0 = 1;
/* ClearBufferMask */
WebGLRenderingContext.prototype.DEPTH_BUFFER_BIT = 0x00000100;
WebGLRenderingContext.prototype.STENCIL_BUFFER_BIT = 0x00000400;
WebGLRenderingContext.prototype.COLOR_BUFFER_BIT = 0x00004000;
/* Boolean */
WebGLRenderingContext.prototype.FALSE = 0;
WebGLRenderingContext.prototype.TRUE = 1;
/* BeginMode */
WebGLRenderingContext.prototype.POINTS = 0x0000;
WebGLRenderingContext.prototype.LINES = 0x0001;
WebGLRenderingContext.prototype.LINE_LOOP = 0x0002;
WebGLRenderingContext.prototype.LINE_STRIP = 0x0003;
WebGLRenderingContext.prototype.TRIANGLES = 0x0004;
WebGLRenderingContext.prototype.TRIANGLE_STRIP = 0x0005;
WebGLRenderingContext.prototype.TRIANGLE_FAN = 0x0006;
/* BlendingFactorDest */
WebGLRenderingContext.prototype.ZERO = 0;
WebGLRenderingContext.prototype.ONE = 1;
WebGLRenderingContext.prototype.SRC_COLOR = 0x0300;
WebGLRenderingContext.prototype.ONE_MINUS_SRC_COLOR = 0x0301;
WebGLRenderingContext.prototype.SRC_ALPHA = 0x0302;
WebGLRenderingContext.prototype.ONE_MINUS_SRC_ALPHA = 0x0303;
WebGLRenderingContext.prototype.DST_ALPHA = 0x0304;
WebGLRenderingContext.prototype.ONE_MINUS_DST_ALPHA = 0x0305;
/* BlendingFactorSrc */
WebGLRenderingContext.prototype.DST_COLOR = 0x0306;
WebGLRenderingContext.prototype.ONE_MINUS_DST_COLOR = 0x0307;
WebGLRenderingContext.prototype.SRC_ALPHA_SATURATE = 0x0308;
/* BlendEquationSeparate */
WebGLRenderingContext.prototype.FUNC_ADD = 0x8006;
WebGLRenderingContext.prototype.BLEND_EQUATION = 0x8009;
WebGLRenderingContext.prototype.BLEND_EQUATION_RGB = 0x8009;
WebGLRenderingContext.prototype.BLEND_EQUATION_ALPHA = 0x883D;
/* BlendSubtract */
WebGLRenderingContext.prototype.FUNC_SUBTRACT = 0x800A;
WebGLRenderingContext.prototype.FUNC_REVERSE_SUBTRACT = 0x800B;
/* Separate Blend Functions */
WebGLRenderingContext.prototype.BLEND_DST_RGB = 0x80C8;
WebGLRenderingContext.prototype.BLEND_SRC_RGB = 0x80C9;
WebGLRenderingContext.prototype.BLEND_DST_ALPHA = 0x80CA;
WebGLRenderingContext.prototype.BLEND_SRC_ALPHA = 0x80CB;
WebGLRenderingContext.prototype.CONSTANT_COLOR = 0x8001;
WebGLRenderingContext.prototype.ONE_MINUS_CONSTANT_COLOR = 0x8002;
WebGLRenderingContext.prototype.CONSTANT_ALPHA = 0x8003;
WebGLRenderingContext.prototype.ONE_MINUS_CONSTANT_ALPHA = 0x8004;
WebGLRenderingContext.prototype.BLEND_COLOR = 0x8005;
/* Buffer Objects */
WebGLRenderingContext.prototype.ARRAY_BUFFER = 0x8892;
WebGLRenderingContext.prototype.ELEMENT_ARRAY_BUFFER = 0x8893;
WebGLRenderingContext.prototype.ARRAY_BUFFER_BINDING = 0x8894;
WebGLRenderingContext.prototype.ELEMENT_ARRAY_BUFFER_BINDING = 0x8895;
WebGLRenderingContext.prototype.STREAM_DRAW = 0x88E0;
WebGLRenderingContext.prototype.STATIC_DRAW = 0x88E4;
WebGLRenderingContext.prototype.DYNAMIC_DRAW = 0x88E8;
WebGLRenderingContext.prototype.BUFFER_SIZE = 0x8764;
WebGLRenderingContext.prototype.BUFFER_USAGE = 0x8765;
WebGLRenderingContext.prototype.CURRENT_VERTEX_ATTRIB = 0x8626;
/* CullFaceMode */
WebGLRenderingContext.prototype.FRONT = 0x0404;
WebGLRenderingContext.prototype.BACK = 0x0405;
WebGLRenderingContext.prototype.FRONT_AND_BACK = 0x0408;
/* EnableCap */
WebGLRenderingContext.prototype.TEXTURE_2D = 0x0DE1;
WebGLRenderingContext.prototype.CULL_FACE = 0x0B44;
WebGLRenderingContext.prototype.BLEND = 0x0BE2;
WebGLRenderingContext.prototype.DITHER = 0x0BD0;
WebGLRenderingContext.prototype.STENCIL_TEST = 0x0B90;
WebGLRenderingContext.prototype.DEPTH_TEST = 0x0B71;
WebGLRenderingContext.prototype.SCISSOR_TEST = 0x0C11;
WebGLRenderingContext.prototype.POLYGON_OFFSET_FILL = 0x8037;
WebGLRenderingContext.prototype.SAMPLE_ALPHA_TO_COVERAGE = 0x809E;
WebGLRenderingContext.prototype.SAMPLE_COVERAGE = 0x80A0;
/* ErrorCode */
WebGLRenderingContext.prototype.NO_ERROR = 0;
WebGLRenderingContext.prototype.INVALID_ENUM = 0x0500;
WebGLRenderingContext.prototype.INVALID_VALUE = 0x0501;
WebGLRenderingContext.prototype.INVALID_OPERATION = 0x0502;
WebGLRenderingContext.prototype.OUT_OF_MEMORY = 0x0505;
/* FrontFaceDirection */
WebGLRenderingContext.prototype.CW = 0x0900;
WebGLRenderingContext.prototype.CCW = 0x0901;
/* GetPName */
WebGLRenderingContext.prototype.LINE_WIDTH = 0x0B21;
WebGLRenderingContext.prototype.ALIASED_POINT_SIZE_RANGE = 0x846D;
WebGLRenderingContext.prototype.ALIASED_LINE_WIDTH_RANGE = 0x846E;
WebGLRenderingContext.prototype.CULL_FACE_MODE = 0x0B45;
WebGLRenderingContext.prototype.FRONT_FACE = 0x0B46;
WebGLRenderingContext.prototype.DEPTH_RANGE = 0x0B70;
WebGLRenderingContext.prototype.DEPTH_WRITEMASK = 0x0B72;
WebGLRenderingContext.prototype.DEPTH_CLEAR_VALUE = 0x0B73;
WebGLRenderingContext.prototype.DEPTH_FUNC = 0x0B74;
WebGLRenderingContext.prototype.STENCIL_CLEAR_VALUE = 0x0B91;
WebGLRenderingContext.prototype.STENCIL_FUNC = 0x0B92;
WebGLRenderingContext.prototype.STENCIL_FAIL = 0x0B94;
WebGLRenderingContext.prototype.STENCIL_PASS_DEPTH_FAIL = 0x0B95;
WebGLRenderingContext.prototype.STENCIL_PASS_DEPTH_PASS = 0x0B96;
WebGLRenderingContext.prototype.STENCIL_REF = 0x0B97;
WebGLRenderingContext.prototype.STENCIL_VALUE_MASK = 0x0B93;
WebGLRenderingContext.prototype.STENCIL_WRITEMASK = 0x0B98;
WebGLRenderingContext.prototype.STENCIL_BACK_FUNC = 0x8800;
WebGLRenderingContext.prototype.STENCIL_BACK_FAIL = 0x8801;
WebGLRenderingContext.prototype.STENCIL_BACK_PASS_DEPTH_FAIL = 0x8802;
WebGLRenderingContext.prototype.STENCIL_BACK_PASS_DEPTH_PASS = 0x8803;
WebGLRenderingContext.prototype.STENCIL_BACK_REF = 0x8CA3;
WebGLRenderingContext.prototype.STENCIL_BACK_VALUE_MASK = 0x8CA4;
WebGLRenderingContext.prototype.STENCIL_BACK_WRITEMASK = 0x8CA5;
WebGLRenderingContext.prototype.VIEWPORT = 0x0BA2;
WebGLRenderingContext.prototype.SCISSOR_BOX = 0x0C10;
WebGLRenderingContext.prototype.COLOR_CLEAR_VALUE = 0x0C22;
WebGLRenderingContext.prototype.COLOR_WRITEMASK = 0x0C23;
WebGLRenderingContext.prototype.UNPACK_ALIGNMENT = 0x0CF5;
WebGLRenderingContext.prototype.PACK_ALIGNMENT = 0x0D05;
WebGLRenderingContext.prototype.MAX_TEXTURE_SIZE = 0x0D33;
WebGLRenderingContext.prototype.MAX_VIEWPORT_DIMS = 0x0D3A;
WebGLRenderingContext.prototype.SUBPIXEL_BITS = 0x0D50;
WebGLRenderingContext.prototype.RED_BITS = 0x0D52;
WebGLRenderingContext.prototype.GREEN_BITS = 0x0D53;
WebGLRenderingContext.prototype.BLUE_BITS = 0x0D54;
WebGLRenderingContext.prototype.ALPHA_BITS = 0x0D55;
WebGLRenderingContext.prototype.DEPTH_BITS = 0x0D56;
WebGLRenderingContext.prototype.STENCIL_BITS = 0x0D57;
WebGLRenderingContext.prototype.POLYGON_OFFSET_UNITS = 0x2A00;
WebGLRenderingContext.prototype.POLYGON_OFFSET_FACTOR = 0x8038;
WebGLRenderingContext.prototype.TEXTURE_BINDING_2D = 0x8069;
WebGLRenderingContext.prototype.SAMPLE_BUFFERS = 0x80A8;
WebGLRenderingContext.prototype.SAMPLES = 0x80A9;
WebGLRenderingContext.prototype.SAMPLE_COVERAGE_VALUE = 0x80AA;
WebGLRenderingContext.prototype.SAMPLE_COVERAGE_INVERT = 0x80AB;
/* GetTextureParameter */
WebGLRenderingContext.prototype.NUM_COMPRESSED_TEXTURE_FORMATS = 0x86A2;
WebGLRenderingContext.prototype.COMPRESSED_TEXTURE_FORMATS = 0x86A3;
/* HintMode */
WebGLRenderingContext.prototype.DONT_CARE = 0x1100;
WebGLRenderingContext.prototype.FASTEST = 0x1101;
WebGLRenderingContext.prototype.NICEST = 0x1102;
/* HintTarget */
WebGLRenderingContext.prototype.GENERATE_MIPMAP_HINT = 0x8192;
/* DataType */
WebGLRenderingContext.prototype.BYTE = 0x1400;
WebGLRenderingContext.prototype.UNSIGNED_BYTE = 0x1401;
WebGLRenderingContext.prototype.SHORT = 0x1402;
WebGLRenderingContext.prototype.UNSIGNED_SHORT = 0x1403;
WebGLRenderingContext.prototype.INT = 0x1404;
WebGLRenderingContext.prototype.UNSIGNED_INT = 0x1405;
WebGLRenderingContext.prototype.FLOAT = 0x1406;
WebGLRenderingContext.prototype.FIXED = 0x140C;
/* PixelFormat */
WebGLRenderingContext.prototype.DEPTH_COMPONENT = 0x1902;
WebGLRenderingContext.prototype.ALPHA = 0x1906;
WebGLRenderingContext.prototype.RGB = 0x1907;
WebGLRenderingContext.prototype.RGBA = 0x1908;
WebGLRenderingContext.prototype.LUMINANCE = 0x1909;
WebGLRenderingContext.prototype.LUMINANCE_ALPHA = 0x190A;
/* PixelType */
WebGLRenderingContext.prototype.UNSIGNED_SHORT_4_4_4_4 = 0x8033;
WebGLRenderingContext.prototype.UNSIGNED_SHORT_5_5_5_1 = 0x8034;
WebGLRenderingContext.prototype.UNSIGNED_SHORT_5_6_5 = 0x8363;
/* Shaders */
WebGLRenderingContext.prototype.FRAGMENT_SHADER = 0x8B30;
WebGLRenderingContext.prototype.VERTEX_SHADER = 0x8B31;
WebGLRenderingContext.prototype.MAX_VERTEX_ATTRIBS = 0x8869;
WebGLRenderingContext.prototype.MAX_VERTEX_UNIFORM_VECTORS = 0x8DFB;
WebGLRenderingContext.prototype.MAX_VARYING_VECTORS = 0x8DFC;
WebGLRenderingContext.prototype.MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D;
WebGLRenderingContext.prototype.MAX_VERTEX_TEXTURE_IMAGE_UNITS = 0x8B4C;
WebGLRenderingContext.prototype.MAX_TEXTURE_IMAGE_UNITS = 0x8872;
WebGLRenderingContext.prototype.MAX_FRAGMENT_UNIFORM_VECTORS = 0x8DFD;
WebGLRenderingContext.prototype.SHADER_TYPE = 0x8B4F;
WebGLRenderingContext.prototype.DELETE_STATUS = 0x8B80;
WebGLRenderingContext.prototype.LINK_STATUS = 0x8B82;
WebGLRenderingContext.prototype.VALIDATE_STATUS = 0x8B83;
WebGLRenderingContext.prototype.ATTACHED_SHADERS = 0x8B85;
WebGLRenderingContext.prototype.ACTIVE_UNIFORMS = 0x8B86;
WebGLRenderingContext.prototype.ACTIVE_UNIFORM_MAX_LENGTH = 0x8B87;
WebGLRenderingContext.prototype.ACTIVE_ATTRIBUTES = 0x8B89;
WebGLRenderingContext.prototype.ACTIVE_ATTRIBUTE_MAX_LENGTH = 0x8B8A;
WebGLRenderingContext.prototype.SHADING_LANGUAGE_VERSION = 0x8B8C;
WebGLRenderingContext.prototype.CURRENT_PROGRAM = 0x8B8D;
/* StencilFunction */
WebGLRenderingContext.prototype.NEVER = 0x0200;
WebGLRenderingContext.prototype.LESS = 0x0201;
WebGLRenderingContext.prototype.EQUAL = 0x0202;
WebGLRenderingContext.prototype.LEQUAL = 0x0203;
WebGLRenderingContext.prototype.GREATER = 0x0204;
WebGLRenderingContext.prototype.NOTEQUAL = 0x0205;
WebGLRenderingContext.prototype.GEQUAL = 0x0206;
WebGLRenderingContext.prototype.ALWAYS = 0x0207;
/* StencilOp */
/*      GL_ZERO */
WebGLRenderingContext.prototype.KEEP = 0x1E00;
WebGLRenderingContext.prototype.REPLACE = 0x1E01;
WebGLRenderingContext.prototype.INCR = 0x1E02;
WebGLRenderingContext.prototype.DECR = 0x1E03;
WebGLRenderingContext.prototype.INVERT = 0x150A;
WebGLRenderingContext.prototype.INCR_WRAP = 0x8507;
WebGLRenderingContext.prototype.DECR_WRAP = 0x8508;
/* StringName */
WebGLRenderingContext.prototype.VENDOR = 0x1F00;
WebGLRenderingContext.prototype.RENDERER = 0x1F01;
WebGLRenderingContext.prototype.VERSION = 0x1F02;
WebGLRenderingContext.prototype.EXTENSIONS = 0x1F03;
/* TextureMagFilter */
WebGLRenderingContext.prototype.NEAREST = 0x2600;
WebGLRenderingContext.prototype.LINEAR = 0x2601;
/* TextureMinFilter */
WebGLRenderingContext.prototype.NEAREST_MIPMAP_NEAREST = 0x2700;
WebGLRenderingContext.prototype.LINEAR_MIPMAP_NEAREST = 0x2701;
WebGLRenderingContext.prototype.NEAREST_MIPMAP_LINEAR = 0x2702;
WebGLRenderingContext.prototype.LINEAR_MIPMAP_LINEAR = 0x2703;
/* TextureParameterName */
WebGLRenderingContext.prototype.TEXTURE_MAG_FILTER = 0x2800;
WebGLRenderingContext.prototype.TEXTURE_MIN_FILTER = 0x2801;
WebGLRenderingContext.prototype.TEXTURE_WRAP_S = 0x2802;
WebGLRenderingContext.prototype.TEXTURE_WRAP_T = 0x2803;
/* TextureTarget */
WebGLRenderingContext.prototype.TEXTURE = 0x1702;
WebGLRenderingContext.prototype.TEXTURE_CUBE_MAP = 0x8513;
WebGLRenderingContext.prototype.TEXTURE_BINDING_CUBE_MAP = 0x8514;
WebGLRenderingContext.prototype.TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515;
WebGLRenderingContext.prototype.TEXTURE_CUBE_MAP_NEGATIVE_X = 0x8516;
WebGLRenderingContext.prototype.TEXTURE_CUBE_MAP_POSITIVE_Y = 0x8517;
WebGLRenderingContext.prototype.TEXTURE_CUBE_MAP_NEGATIVE_Y = 0x8518;
WebGLRenderingContext.prototype.TEXTURE_CUBE_MAP_POSITIVE_Z = 0x8519;
WebGLRenderingContext.prototype.TEXTURE_CUBE_MAP_NEGATIVE_Z = 0x851A;
WebGLRenderingContext.prototype.MAX_CUBE_MAP_TEXTURE_SIZE = 0x851C;
/* TextureUnit */
WebGLRenderingContext.prototype.TEXTURE0 = 0x84C0;
WebGLRenderingContext.prototype.TEXTURE1 = 0x84C1;
WebGLRenderingContext.prototype.TEXTURE2 = 0x84C2;
WebGLRenderingContext.prototype.TEXTURE3 = 0x84C3;
WebGLRenderingContext.prototype.TEXTURE4 = 0x84C4;
WebGLRenderingContext.prototype.TEXTURE5 = 0x84C5;
WebGLRenderingContext.prototype.TEXTURE6 = 0x84C6;
WebGLRenderingContext.prototype.TEXTURE7 = 0x84C7;
WebGLRenderingContext.prototype.TEXTURE8 = 0x84C8;
WebGLRenderingContext.prototype.TEXTURE9 = 0x84C9;
WebGLRenderingContext.prototype.TEXTURE10 = 0x84CA;
WebGLRenderingContext.prototype.TEXTURE11 = 0x84CB;
WebGLRenderingContext.prototype.TEXTURE12 = 0x84CC;
WebGLRenderingContext.prototype.TEXTURE13 = 0x84CD;
WebGLRenderingContext.prototype.TEXTURE14 = 0x84CE;
WebGLRenderingContext.prototype.TEXTURE15 = 0x84CF;
WebGLRenderingContext.prototype.TEXTURE16 = 0x84D0;
WebGLRenderingContext.prototype.TEXTURE17 = 0x84D1;
WebGLRenderingContext.prototype.TEXTURE18 = 0x84D2;
WebGLRenderingContext.prototype.TEXTURE19 = 0x84D3;
WebGLRenderingContext.prototype.TEXTURE20 = 0x84D4;
WebGLRenderingContext.prototype.TEXTURE21 = 0x84D5;
WebGLRenderingContext.prototype.TEXTURE22 = 0x84D6;
WebGLRenderingContext.prototype.TEXTURE23 = 0x84D7;
WebGLRenderingContext.prototype.TEXTURE24 = 0x84D8;
WebGLRenderingContext.prototype.TEXTURE25 = 0x84D9;
WebGLRenderingContext.prototype.TEXTURE26 = 0x84DA;
WebGLRenderingContext.prototype.TEXTURE27 = 0x84DB;
WebGLRenderingContext.prototype.TEXTURE28 = 0x84DC;
WebGLRenderingContext.prototype.TEXTURE29 = 0x84DD;
WebGLRenderingContext.prototype.TEXTURE30 = 0x84DE;
WebGLRenderingContext.prototype.TEXTURE31 = 0x84DF;
WebGLRenderingContext.prototype.ACTIVE_TEXTURE = 0x84E0;
/* TextureWrapMode */
WebGLRenderingContext.prototype.REPEAT = 0x2901;
WebGLRenderingContext.prototype.CLAMP_TO_EDGE = 0x812F;
WebGLRenderingContext.prototype.MIRRORED_REPEAT = 0x8370;
/* Uniform Types */
WebGLRenderingContext.prototype.FLOAT_VEC2 = 0x8B50;
WebGLRenderingContext.prototype.FLOAT_VEC3 = 0x8B51;
WebGLRenderingContext.prototype.FLOAT_VEC4 = 0x8B52;
WebGLRenderingContext.prototype.INT_VEC2 = 0x8B53;
WebGLRenderingContext.prototype.INT_VEC3 = 0x8B54;
WebGLRenderingContext.prototype.INT_VEC4 = 0x8B55;
WebGLRenderingContext.prototype.BOOL = 0x8B56;
WebGLRenderingContext.prototype.BOOL_VEC2 = 0x8B57;
WebGLRenderingContext.prototype.BOOL_VEC3 = 0x8B58;
WebGLRenderingContext.prototype.BOOL_VEC4 = 0x8B59;
WebGLRenderingContext.prototype.FLOAT_MAT2 = 0x8B5A;
WebGLRenderingContext.prototype.FLOAT_MAT3 = 0x8B5B;
WebGLRenderingContext.prototype.FLOAT_MAT4 = 0x8B5C;
WebGLRenderingContext.prototype.SAMPLER_2D = 0x8B5E;
WebGLRenderingContext.prototype.SAMPLER_CUBE = 0x8B60;
/* Vertex Arrays */
WebGLRenderingContext.prototype.VERTEX_ATTRIB_ARRAY_ENABLED = 0x8622;
WebGLRenderingContext.prototype.VERTEX_ATTRIB_ARRAY_SIZE = 0x8623;
WebGLRenderingContext.prototype.VERTEX_ATTRIB_ARRAY_STRIDE = 0x8624;
WebGLRenderingContext.prototype.VERTEX_ATTRIB_ARRAY_TYPE = 0x8625;
WebGLRenderingContext.prototype.VERTEX_ATTRIB_ARRAY_NORMALIZED = 0x886A;
WebGLRenderingContext.prototype.VERTEX_ATTRIB_ARRAY_POINTER = 0x8645;
WebGLRenderingContext.prototype.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F;
/* Read Format */
WebGLRenderingContext.prototype.IMPLEMENTATION_COLOR_READ_TYPE = 0x8B9A;
WebGLRenderingContext.prototype.IMPLEMENTATION_COLOR_READ_FORMAT = 0x8B9B;
/* Shader Source */
WebGLRenderingContext.prototype.COMPILE_STATUS = 0x8B81;
WebGLRenderingContext.prototype.INFO_LOG_LENGTH = 0x8B84;
WebGLRenderingContext.prototype.SHADER_SOURCE_LENGTH = 0x8B88;
WebGLRenderingContext.prototype.SHADER_COMPILER = 0x8DFA;
/* Shader Binary */
WebGLRenderingContext.prototype.SHADER_BINARY_FORMATS = 0x8DF8;
WebGLRenderingContext.prototype.NUM_SHADER_BINARY_FORMATS = 0x8DF9;
/* Shader Precision-Specified Types */
WebGLRenderingContext.prototype.LOW_FLOAT = 0x8DF0;
WebGLRenderingContext.prototype.MEDIUM_FLOAT = 0x8DF1;
WebGLRenderingContext.prototype.HIGH_FLOAT = 0x8DF2;
WebGLRenderingContext.prototype.LOW_INT = 0x8DF3;
WebGLRenderingContext.prototype.MEDIUM_INT = 0x8DF4;
WebGLRenderingContext.prototype.HIGH_INT = 0x8DF5;
/* Framebuffer Object. */
WebGLRenderingContext.prototype.FRAMEBUFFER = 0x8D40;
WebGLRenderingContext.prototype.RENDERBUFFER = 0x8D41;
WebGLRenderingContext.prototype.RGBA4 = 0x8056;
WebGLRenderingContext.prototype.RGB5_A1 = 0x8057;
WebGLRenderingContext.prototype.RGB565 = 0x8D62;
WebGLRenderingContext.prototype.DEPTH_COMPONENT16 = 0x81A5;
WebGLRenderingContext.prototype.STENCIL_INDEX = 0x1901;
WebGLRenderingContext.prototype.STENCIL_INDEX8 = 0x8D48;
WebGLRenderingContext.prototype.RENDERBUFFER_WIDTH = 0x8D42;
WebGLRenderingContext.prototype.RENDERBUFFER_HEIGHT = 0x8D43;
WebGLRenderingContext.prototype.RENDERBUFFER_INTERNAL_FORMAT = 0x8D44;
WebGLRenderingContext.prototype.RENDERBUFFER_RED_SIZE = 0x8D50;
WebGLRenderingContext.prototype.RENDERBUFFER_GREEN_SIZE = 0x8D51;
WebGLRenderingContext.prototype.RENDERBUFFER_BLUE_SIZE = 0x8D52;
WebGLRenderingContext.prototype.RENDERBUFFER_ALPHA_SIZE = 0x8D53;
WebGLRenderingContext.prototype.RENDERBUFFER_DEPTH_SIZE = 0x8D54;
WebGLRenderingContext.prototype.RENDERBUFFER_STENCIL_SIZE = 0x8D55;
WebGLRenderingContext.prototype.FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE = 0x8CD0;
WebGLRenderingContext.prototype.FRAMEBUFFER_ATTACHMENT_OBJECT_NAME = 0x8CD1;
WebGLRenderingContext.prototype.FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL = 0x8CD2;
WebGLRenderingContext.prototype.FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3;
WebGLRenderingContext.prototype.COLOR_ATTACHMENT0 = 0x8CE0;
WebGLRenderingContext.prototype.DEPTH_ATTACHMENT = 0x8D00;
WebGLRenderingContext.prototype.STENCIL_ATTACHMENT = 0x8D20;
WebGLRenderingContext.prototype.NONE = 0;
WebGLRenderingContext.prototype.FRAMEBUFFER_COMPLETE = 0x8CD5;
WebGLRenderingContext.prototype.FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6;
WebGLRenderingContext.prototype.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7;
WebGLRenderingContext.prototype.FRAMEBUFFER_INCOMPLETE_DIMENSIONS = 0x8CD9;
WebGLRenderingContext.prototype.FRAMEBUFFER_UNSUPPORTED = 0x8CDD;
WebGLRenderingContext.prototype.FRAMEBUFFER_BINDING = 0x8CA6;
WebGLRenderingContext.prototype.RENDERBUFFER_BINDING = 0x8CA7;
WebGLRenderingContext.prototype.MAX_RENDERBUFFER_SIZE = 0x84E8;
WebGLRenderingContext.prototype.INVALID_FRAMEBUFFER_OPERATION = 0x0506;

/* WebGL-specific enums */
WebGLRenderingContext.prototype.UNPACK_FLIP_Y_WEBGL = 0x9240;
WebGLRenderingContext.prototype.UNPACK_PREMULTIPLY_ALPHA_WEBGL = 0x9241;

/* Non-WebGL enums: pixel format that switches B and R values; handy when using cairo canvas. */
WebGLRenderingContext.prototype.UNPACK_FLIP_BLUE_RED = 0x9245;

/* WebGL-specific not supported enums */
//WebGLRenderingContext.prototype.CONTEXT_LOST_WEBGL = 0x9242;
//WebGLRenderingContext.prototype.UNPACK_COLORSPACE_CONVERSION_WEBGL = 0x9243;
//WebGLRenderingContext.prototype.BROWSER_DEFAULT_WEBGL = 0x9244;


WebGLRenderingContext.prototype.getSupportedExtensions = function getSupportedExtensions() {
    return this.gl.getSupportedExtensions().split(" ");
};

WebGLRenderingContext.prototype.getExtension = function getExtension(name) {
    if (!(arguments.length === 1 && typeof name === "string")) {
        throw new TypeError('Expected getExtension(string name)');
    }
    return this.gl.getExtension(name);
};

WebGLRenderingContext.prototype.activeTexture = function activeTexture(texture) {
    if (!(arguments.length === 1 && typeof texture === "number")) {
        throw new TypeError('Expected activeTexture(number texture)');
    }
    return this.gl.activeTexture(texture);
};

WebGLRenderingContext.prototype.attachShader = function attachShader(program, shader) {
    if (!(arguments.length === 2 && (program === null || program instanceof WebGLProgram) && (shader === null || shader instanceof WebGLShader))) {
        throw new TypeError('Expected attachShader(WebGLProgram program, WebGLShader shader)');
    }
    return this.gl.attachShader(program ? program._ : 0, shader ? shader._ : 0);
};

WebGLRenderingContext.prototype.bindAttribLocation = function bindAttribLocation(program, index, name) {
    if (!(arguments.length === 3 && (program === null || program instanceof WebGLProgram) && typeof index === "number" && typeof name === "string")) {
        throw new TypeError('Expected bindAttribLocation(WebGLProgram program, number index, string name)');
    }
    return this.gl.bindAttribLocation(program ? program._ : 0, index, name);
};

WebGLRenderingContext.prototype.bindBuffer = function bindBuffer(target, buffer) {
    if (!(arguments.length === 2 && typeof target === "number" && (buffer === null || buffer instanceof WebGLBuffer))) {
        throw new TypeError('Expected bindBuffer(number target, WebGLBuffer buffer)');
    }
    return this.gl.bindBuffer(target, buffer ? buffer._ : 0);
};

WebGLRenderingContext.prototype.bindFramebuffer = function bindFramebuffer(target, framebuffer) {
    if (!(arguments.length === 2 && typeof target === "number" && (framebuffer === null || framebuffer instanceof WebGLFramebuffer))) {
        throw new TypeError('Expected bindFramebuffer(number target, WebGLFramebuffer framebuffer)');
    }
    return this.gl.bindFramebuffer(target, framebuffer ? framebuffer._ : 0);
};

WebGLRenderingContext.prototype.bindRenderbuffer = function bindRenderbuffer(target, renderbuffer) {
    if (!(arguments.length === 2 && typeof target === "number" && (renderbuffer === null || renderbuffer instanceof WebGLRenderbuffer))) {
        throw new TypeError('Expected bindRenderbuffer(number target, WebGLRenderbuffer renderbuffer)');
    }
    return this.gl.bindRenderbuffer(target, renderbuffer ? renderbuffer._ : 0);
};

WebGLRenderingContext.prototype.bindTexture = function bindTexture(target, texture) {
    if (!(arguments.length === 2 && typeof target === "number" && (texture === null || texture instanceof WebGLTexture))) {
        throw new TypeError('Expected bindTexture(number target, WebGLTexture texture)');
    }
    return this.gl.bindTexture(target, texture ? texture._ : 0);
};

WebGLRenderingContext.prototype.blendColor = function blendColor(red, green, blue, alpha) {
    if (!(arguments.length === 4 && typeof red === "number" && typeof green === "number" && typeof blue === "number" && typeof alpha === "number")) {
        throw new TypeError('Expected blendColor(number red, number green, number blue, number alpha)');
    }
    return this.gl.blendColor(red, green, blue, alpha);
};

WebGLRenderingContext.prototype.blendEquation = function blendEquation(mode) {
    if (!(arguments.length === 1 && typeof mode === "number")) {
        throw new TypeError('Expected blendEquation(number mode)');
    }
    return this.gl.blendEquation(mode);
};

WebGLRenderingContext.prototype.blendEquationSeparate = function blendEquationSeparate(modeRGB, modeAlpha) {
    if (!(arguments.length === 2 && typeof modeRGB === "number" && typeof modeAlpha === "number")) {
        throw new TypeError('Expected blendEquationSeparate(number modeRGB, number modeAlpha)');
    }
    return this.gl.blendEquationSeparate(modeRGB, modeAlpha);
};

WebGLRenderingContext.prototype.blendFunc = function blendFunc(sfactor, dfactor) {
    if (!(arguments.length === 2 && typeof sfactor === "number" && typeof dfactor === "number")) {
        throw new TypeError('Expected blendFunc(number sfactor, number dfactor)');
    }
    return this.gl.blendFunc(sfactor, dfactor);
};

WebGLRenderingContext.prototype.blendFuncSeparate = function blendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha) {
    if (!(arguments.length === 4 && typeof srcRGB === "number" && typeof dstRGB === "number" && typeof srcAlpha === "number" && typeof dstAlpha === "number")) {
        throw new TypeError('Expected blendFuncSeparate(number srcRGB, number dstRGB, number srcAlpha, number dstAlpha)');
    }
    return this.gl.blendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
};

WebGLRenderingContext.prototype.bufferData = function bufferData(target, data, usage) {
    if (!(arguments.length === 3 && typeof target === "number" &&
        (typeof data === "object" || typeof data === "number") && typeof usage === "number")) {
        throw new TypeError('Expected bufferData(number target, ArrayBuffer data, number usage) or bufferData(number target, number size, number usage)');
    }
    return this.gl.bufferData(target, data, usage);
};

WebGLRenderingContext.prototype.bufferSubData = function bufferSubData(target, offset, data) {
    if (!(arguments.length === 3 && typeof target === "number" && typeof offset === "number" && typeof data === "object")) {
        throw new TypeError('Expected bufferSubData(number target, number offset, ArrayBuffer data)');
    }
    return this.gl.bufferSubData(target, offset, data);
};

WebGLRenderingContext.prototype.checkFramebufferStatus = function checkFramebufferStatus(target) {
    if (!(arguments.length === 1 && typeof target === "number")) {
        throw new TypeError('Expected checkFramebufferStatus(number target)');
    }
    return this.gl.checkFramebufferStatus(target);
};

WebGLRenderingContext.prototype.clear = function clear(mask) {
    if (!(arguments.length === 1 && typeof mask === "number")) {
        throw new TypeError('Expected clear(number mask)');
    }
    return this.gl.clear(mask);
};

WebGLRenderingContext.prototype.clearColor = function clearColor(red, green, blue, alpha) {
    if (!(arguments.length === 4 && typeof red === "number" && typeof green === "number" && typeof blue === "number" && typeof alpha === "number")) {
        throw new TypeError('Expected clearColor(number red, number green, number blue, number alpha)');
    }
    return this.gl.clearColor(red, green, blue, alpha);
};

WebGLRenderingContext.prototype.clearDepth = function clearDepth(depth) {
    if (!(arguments.length === 1 && typeof depth === "number")) {
        throw new TypeError('Expected clearDepth(number depth)');
    }
    return this.gl.clearDepth(depth);
};

WebGLRenderingContext.prototype.clearStencil = function clearStencil(s) {
    if (!(arguments.length === 1 && typeof s === "number")) {
        throw new TypeError('Expected clearStencil(number s)');
    }
    return this.gl.clearStencil(s);
};

WebGLRenderingContext.prototype.colorMask = function colorMask(red, green, blue, alpha) {
    if (!(arguments.length === 4 && typeof red === "boolean" && typeof green === "boolean" && typeof blue === "boolean" && typeof alpha === "boolean")) {
        throw new TypeError('Expected colorMask(boolean red, boolean green, boolean blue, boolean alpha)');
    }
    return this.gl.colorMask(red, green, blue, alpha);
};

WebGLRenderingContext.prototype.compileShader = function compileShader(shader) {
    if (!(arguments.length === 1 && (shader === null || shader instanceof WebGLShader))) {
        throw new TypeError('Expected compileShader(WebGLShader shader)');
    }
    return this.gl.compileShader(shader ? shader._ : 0);
};

WebGLRenderingContext.prototype.copyTexImage2D = function copyTexImage2D(target, level, internalformat, x, y, width, height, border) {
    if (!(arguments.length === 8 && typeof target === "number" && typeof level === "number" && typeof internalformat === "number" && typeof x === "number" && typeof y === "number" && typeof width === "number" && typeof height === "number" && typeof border === "number")) {
        throw new TypeError('Expected copyTexImage2D(number target, number level, number internalformat, number x, number y, number width, number height, number border)');
    }
    return this.gl.copyTexImage2D(target, level, internalformat, x, y, width, height, border);
};

WebGLRenderingContext.prototype.copyTexSubImage2D = function copyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height) {
    if (!(arguments.length === 8 && typeof target === "number" && typeof level === "number" && typeof xoffset === "number" && typeof yoffset === "number" && typeof x === "number" && typeof y === "number" && typeof width === "number" && typeof height === "number")) {
        throw new TypeError('Expected copyTexSubImage2D(number target, number level, number xoffset, number yoffset, number x, number y, number width, number height)');
    }
    return this.gl.copyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
};

WebGLRenderingContext.prototype.createBuffer = function createBuffer() {
    if (!(arguments.length === 0)) {
        throw new TypeError('Expected createBuffer()');
    }
    return new WebGLBuffer(this.gl.createBuffer());
};

WebGLRenderingContext.prototype.createFramebuffer = function () {
    if (!(arguments.length === 0)) {
        throw new TypeError('Expected createFramebuffer()');
    }
    return new WebGLFramebuffer(this.gl.createFramebuffer());
};

WebGLRenderingContext.prototype.createProgram = function createProgram() {
    if (!(arguments.length === 0)) {
        throw new TypeError('Expected createProgram()');
    }
    return new WebGLProgram(this.gl.createProgram());
};

WebGLRenderingContext.prototype.createRenderbuffer = function createRenderbuffer() {
    if (!(arguments.length === 0)) {
        throw new TypeError('Expected createRenderbuffer()');
    }
    return new WebGLRenderbuffer(this.gl.createRenderbuffer());
};

WebGLRenderingContext.prototype.createShader = function createShader(type) {
    if (!(arguments.length === 1 && typeof type === "number")) {
        throw new TypeError('Expected createShader(number type)');
    }
    return new WebGLShader(this.gl.createShader(type));
};

WebGLRenderingContext.prototype.createTexture = function createTexture() {
    if (!(arguments.length === 0)) {
        throw new TypeError('Expected createTexture()');
    }
    return new WebGLTexture(this.gl.createTexture());
};

WebGLRenderingContext.prototype.cullFace = function cullFace(mode) {
    if (!(arguments.length === 1 && typeof mode === "number")) {
        throw new TypeError('Expected cullFace(number mode)');
    }
    return this.gl.cullFace(mode);
};

WebGLRenderingContext.prototype.deleteBuffer = function deleteBuffer(buffer) {
    if (!(arguments.length === 1 && (buffer === null || buffer instanceof WebGLBuffer))) {
        throw new TypeError('Expected deleteBuffer(WebGLBuffer buffer)');
    }
    return this.gl.deleteBuffer(buffer ? buffer._ : 0);
};

WebGLRenderingContext.prototype.deleteFramebuffer = function deleteFramebuffer(framebuffer) {
    if (!(arguments.length === 1 && (framebuffer === null || framebuffer instanceof WebGLFramebuffer))) {
        throw new TypeError('Expected deleteFramebuffer(WebGLFramebuffer framebuffer)');
    }
    return this.gl.deleteFramebuffer(framebuffer ? framebuffer._ : 0);
};

WebGLRenderingContext.prototype.deleteProgram = function deleteProgram(program) {
    if (!(arguments.length === 1 && (program === null || program instanceof WebGLProgram))) {
        throw new TypeError('Expected deleteProgram(WebGLProgram program)');
    }
    return this.gl.deleteProgram(program ? program._ : 0);
};

WebGLRenderingContext.prototype.deleteRenderbuffer = function deleteRenderbuffer(renderbuffer) {
    if (!(arguments.length === 1 && (renderbuffer === null || renderbuffer instanceof WebGLRenderbuffer))) {
        throw new TypeError('Expected deleteRenderbuffer(WebGLRenderbuffer renderbuffer)');
    }
    return this.gl.deleteRenderbuffer(renderbuffer ? renderbuffer._ : 0);
};

WebGLRenderingContext.prototype.deleteShader = function deleteShader(shader) {
    if (!(arguments.length === 1 && (shader === null || shader instanceof WebGLShader))) {
        throw new TypeError('Expected deleteShader(WebGLShader shader)');
    }
    return this.gl.deleteShader(shader ? shader._ : 0);
};

WebGLRenderingContext.prototype.deleteTexture = function deleteTexture(texture) {
    if (!(arguments.length === 1 && (texture === null || texture instanceof WebGLTexture))) {
        throw new TypeError('Expected deleteTexture(WebGLTexture texture)');
    }
    return this.gl.deleteTexture(texture ? texture._ : 0);
};

WebGLRenderingContext.prototype.depthFunc = function depthFunc(func) {
    if (!(arguments.length === 1 && typeof func === "number")) {
        throw new TypeError('Expected depthFunc(number func)');
    }
    return this.gl.depthFunc(func);
};

WebGLRenderingContext.prototype.depthMask = function depthMask(flag) {
    if (!(arguments.length === 1 && typeof flag === "boolean")) {
        throw new TypeError('Expected depthMask(boolean flag)');
    }
    return this.gl.depthMask(flag);
};

WebGLRenderingContext.prototype.depthRange = function depthRange(zNear, zFar) {
    if (!(arguments.length === 2 && typeof zNear === "number" && typeof zFar === "number")) {
        throw new TypeError('Expected depthRange(number zNear, number zFar)');
    }
    return this.gl.depthRange(zNear, zFar);
};

WebGLRenderingContext.prototype.detachShader = function detachShader(program, shader) {
    if (!(arguments.length === 2 && (program === null || program instanceof WebGLProgram) && (shader === null || shader instanceof WebGLShader))) {
        throw new TypeError('Expected detachShader(WebGLProgram program, WebGLShader shader)');
    }
    return this.gl.detachShader(program ? program._ : 0, shader ? shader._ : 0);
};

WebGLRenderingContext.prototype.disable = function disable(cap) {
    if (!(arguments.length === 1 && typeof cap === "number")) {
        throw new TypeError('Expected disable(number cap)');
    }
    return this.gl.disable(cap);
};

WebGLRenderingContext.prototype.disableVertexAttribArray = function disableVertexAttribArray(index) {
    if (!(arguments.length === 1 && typeof index === "number")) {
        throw new TypeError('Expected disableVertexAttribArray(number index)');
    }
    return this.gl.disableVertexAttribArray(index);
};

WebGLRenderingContext.prototype.drawArrays = function drawArrays(mode, first, count) {
    if (!(arguments.length === 3 && typeof mode === "number" && typeof first === "number" && typeof count === "number")) {
        throw new TypeError('Expected drawArrays(number mode, number first, number count)');
    }
    return this.gl.drawArrays(mode, first, count);
};

WebGLRenderingContext.prototype.drawElements = function drawElements(mode, count, type, offset) {
    if (!(arguments.length === 4 && typeof mode === "number" && typeof count === "number" && typeof type === "number" && typeof offset === "number")) {
        throw new TypeError('Expected drawElements(number mode, number count, number type, number offset)');
    }
    return this.gl.drawElements(mode, count, type, offset);
};

WebGLRenderingContext.prototype.enable = function enable(cap) {
    if (!(arguments.length === 1 && typeof cap === "number")) {
        throw new TypeError('Expected enable(number cap)');
    }
    return this.gl.enable(cap);
};

WebGLRenderingContext.prototype.enableVertexAttribArray = function enableVertexAttribArray(index) {
    if (!(arguments.length === 1 && typeof index === "number")) {
        throw new TypeError('Expected enableVertexAttribArray(number index)');
    }
    return this.gl.enableVertexAttribArray(index);
};

WebGLRenderingContext.prototype.finish = function finish() {
    if (!(arguments.length === 0)) {
        throw new TypeError('Expected finish()');
    }
    return this.gl.finish();
};

WebGLRenderingContext.prototype.flush = function flush() {
    if (!(arguments.length === 0)) {
        throw new TypeError('Expected flush()');
    }
    return this.gl.flush();
};

WebGLRenderingContext.prototype.framebufferRenderbuffer = function framebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer) {
    if (!(arguments.length === 4 && typeof target === "number" && typeof attachment === "number" && typeof renderbuffertarget === "number" && (renderbuffer === null || renderbuffer instanceof WebGLRenderbuffer))) {
        throw new TypeError('Expected framebufferRenderbuffer(number target, number attachment, number renderbuffertarget, WebGLRenderbuffer renderbuffer)');
    }
    return this.gl.framebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer ? renderbuffer._ : 0);
};

WebGLRenderingContext.prototype.framebufferTexture2D = function framebufferTexture2D(target, attachment, textarget, texture, level) {
    if (!(arguments.length === 5 && typeof target === "number" && typeof attachment === "number" && typeof textarget === "number" && (texture === null || texture instanceof WebGLTexture) && typeof level === "number")) {
        throw new TypeError('Expected framebufferTexture2D(number target, number attachment, number textarget, WebGLTexture texture, number level)');
    }
    return this.gl.framebufferTexture2D(target, attachment, textarget, texture ? texture._ : 0, level);
};

WebGLRenderingContext.prototype.frontFace = function frontFace(mode) {
    if (!(arguments.length === 1 && typeof mode === "number")) {
        throw new TypeError('Expected frontFace(number mode)');
    }
    return this.gl.frontFace(mode);
};

WebGLRenderingContext.prototype.generateMipmap = function generateMipmap(target) {
    if (!(arguments.length === 1 && typeof target === "number")) {
        throw new TypeError('Expected generateMipmap(number target)');
    }
    return this.gl.generateMipmap(target);
};

WebGLRenderingContext.prototype.getActiveAttrib = function getActiveAttrib(program, index) {
    if (!(arguments.length === 2 && (program === null || program instanceof WebGLProgram) && typeof index === "number")) {
        throw new TypeError('Expected getActiveAttrib(WebGLProgram program, number index)');
    }
    return new WebGLActiveInfo(this.gl.getActiveAttrib(program ? program._ : 0, index));
};

WebGLRenderingContext.prototype.getActiveUniform = function getActiveUniform(program, index) {
    if (!(arguments.length === 2 && (program === null || program instanceof WebGLProgram) && typeof index === "number")) {
        throw new TypeError('Expected getActiveUniform(WebGLProgram program, number index)');
    }
    return new WebGLActiveInfo(this.gl.getActiveUniform(program ? program._ : 0, index));
};

WebGLRenderingContext.prototype.getAttachedShaders = function getAttachedShaders(program) {
    if (!(arguments.length === 1 && (program === null || program instanceof WebGLProgram))) {
        throw new TypeError('Expected getAttachedShaders(WebGLProgram program)');
    }
    return this.gl.getAttachedShaders(program ? program._ : 0);
};

WebGLRenderingContext.prototype.getAttribLocation = function getAttribLocation(program, name) {
    if (!(arguments.length === 2 && (program === null || program instanceof WebGLProgram) && typeof name === "string")) {
        throw new TypeError('Expected getAttribLocation(WebGLProgram program, string name)');
    }
    return this.gl.getAttribLocation(program ? program._ : 0, name);
};

WebGLRenderingContext.prototype.getParameter = function getParameter(pname) {
    if (!(arguments.length === 1 && typeof pname === "number")) {
        throw new TypeError('Expected getParameter(number pname)');
    }
    return this.gl.getParameter(pname);
};

WebGLRenderingContext.prototype.getBufferParameter = function getBufferParameter(target, pname) {
    if (!(arguments.length === 2 && typeof target === "number" && typeof pname === "number")) {
        throw new TypeError('Expected getBufferParameter(number target, number pname)');
    }
    return this.gl.getBufferParameter(target, pname);
};

WebGLRenderingContext.prototype.getError = function getError() {
    if (!(arguments.length === 0)) {
        throw new TypeError('Expected getError()');
    }
    return this.gl.getError();
};

WebGLRenderingContext.prototype.getFramebufferAttachmentParameter = function getFramebufferAttachmentParameter(target, attachment, pname) {
    if (!(arguments.length === 3 && typeof target === "number" && typeof attachment === "number" && typeof pname === "number")) {
        throw new TypeError('Expected getFramebufferAttachmentParameter(number target, number attachment, number pname)');
    }
    return this.gl.getFramebufferAttachmentParameter(target, attachment, pname);
};

WebGLRenderingContext.prototype.getProgramParameter = function getProgramParameter(program, pname) {
    if (!(arguments.length === 2 && (program === null || program instanceof WebGLProgram) && typeof pname === "number")) {
        throw new TypeError('Expected getProgramParameter(WebGLProgram program, number pname)');
    }
    return this.gl.getProgramParameter(program ? program._ : 0, pname);
};

WebGLRenderingContext.prototype.getProgramInfoLog = function getProgramInfoLog(program) {
    if (!(arguments.length === 1 && (program === null || program instanceof WebGLProgram))) {
        throw new TypeError('Expected getProgramInfoLog(WebGLProgram program)');
    }
    return this.gl.getProgramInfoLog(program ? program._ : 0);
};

WebGLRenderingContext.prototype.getRenderbufferParameter = function getRenderbufferParameter(target, pname) {
    if (!(arguments.length === 2 && typeof target === "number" && typeof pname === "number")) {
        throw new TypeError('Expected getRenderbufferParameter(number target, number pname)');
    }
    return this.gl.getRenderbufferParameter(target, pname);
};

WebGLRenderingContext.prototype.getShaderParameter = function getShaderParameter(shader, pname) {
    if (!(arguments.length === 2 && (shader === null || shader instanceof WebGLShader) && typeof pname === "number")) {
        throw new TypeError('Expected getShaderParameter(WebGLShader shader, number pname)');
    }
    return this.gl.getShaderParameter(shader ? shader._ : 0, pname);
};

WebGLRenderingContext.prototype.getShaderInfoLog = function getShaderInfoLog(shader) {
    if (!(arguments.length === 1 && (shader === null || shader instanceof WebGLShader))) {
        throw new TypeError('Expected getShaderInfoLog(WebGLShader shader)');
    }
    return this.gl.getShaderInfoLog(shader ? shader._ : 0);
};

WebGLRenderingContext.prototype.getShaderSource = function getShaderSource(shader) {
    if (!(arguments.length === 1 && (shader === null || shader instanceof WebGLShader))) {
        throw new TypeError('Expected getShaderSource(WebGLShader shader)');
    }
    return this.gl.getShaderSource(shader ? shader._ : 0);
};

WebGLRenderingContext.prototype.getTexParameter = function getTexParameter(target, pname) {
    if (!(arguments.length === 2 && typeof target === "number" && typeof pname === "number")) {
        throw new TypeError('Expected getTexParameter(number target, number pname)');
    }
    return this.gl.getTexParameter(target, pname);
};

WebGLRenderingContext.prototype.getUniform = function getUniform(program, location) {
    if (!(arguments.length === 2 && (program === null || program instanceof WebGLProgram) && (location === null || location instanceof WebGLUniformLocation))) {
        throw new TypeError('Expected getUniform(WebGLProgram program, WebGLUniformLocation location)');
    }
    return this.gl.getUniform(program ? program._ : 0, location ? location._ : 0);
};

WebGLRenderingContext.prototype.getUniformLocation = function getUniformLocation(program, name) {
    if (!(arguments.length === 2 && (program === null || program instanceof WebGLProgram) && typeof name === "string")) {
        throw new TypeError('Expected getUniformLocation(WebGLProgram program, string name)');
    }
    return new WebGLUniformLocation(this.gl.getUniformLocation(program ? program._ : 0, name));
};

WebGLRenderingContext.prototype.getVertexAttrib = function getVertexAttrib(index, pname) {
    if (!(arguments.length === 2 && typeof index === "number" && typeof pname === "number")) {
        throw new TypeError('Expected getVertexAttrib(number index, number pname)');
    }
    return this.gl.getVertexAttrib(index, pname);
};

WebGLRenderingContext.prototype.getVertexAttribOffset = function getVertexAttribOffset(index, pname) {
    if (!(arguments.length === 2 && typeof index === "number" && typeof pname === "number")) {
        throw new TypeError('Expected getVertexAttribOffset(number index, number pname)');
    }

    if(pname === WebGLRenderingContext.CURRENT_VERTEX_ATTRIB) {
        var buf=this.gl.getVertexAttribOffset(index, pname);
        return new Float32Array(buf);
    }
    return this.gl.getVertexAttribOffset(index, pname);
};

WebGLRenderingContext.prototype.hint = function hint(target, mode) {
    if (!(arguments.length === 2 && typeof target === "number" && typeof mode === "number")) {
        throw new TypeError('Expected hint(number target, number mode)');
    }
    return this.gl.hint(target, mode);
};

WebGLRenderingContext.prototype.isBuffer = function isBuffer(buffer) {
    if (!(arguments.length === 1 && (buffer === null || buffer instanceof WebGLBuffer))) {
        throw new TypeError('Expected isBuffer(WebGLBuffer buffer)');
    }
    return this.gl.isBuffer(buffer ? buffer._ : 0);
};

WebGLRenderingContext.prototype.isEnabled = function isEnabled(cap) {
    if (!(arguments.length === 1 && typeof cap === "number")) {
        throw new TypeError('Expected isEnabled(number cap)');
    }
    return this.gl.isEnabled(cap);
};

WebGLRenderingContext.prototype.isFramebuffer = function isFramebuffer(framebuffer) {
    if (!(arguments.length === 1 && (framebuffer === null || framebuffer instanceof WebGLFramebuffer))) {
        throw new TypeError('Expected isFramebuffer(WebGLFramebuffer framebuffer)');
    }
    return this.gl.isFramebuffer(framebuffer ? framebuffer._ : 0);
};

WebGLRenderingContext.prototype.isProgram = function isProgram(program) {
    if (!(arguments.length === 1 && (program === null || program instanceof WebGLProgram))) {
        throw new TypeError('Expected isProgram(WebGLProgram program)');
    }
    return this.gl.isProgram(program ? program._ : 0);
};

WebGLRenderingContext.prototype.isRenderbuffer = function isRenderbuffer(renderbuffer) {
    if (!(arguments.length === 1 && (renderbuffer === null || renderbuffer instanceof WebGLRenderbuffer))) {
        throw new TypeError('Expected isRenderbuffer(WebGLRenderbuffer renderbuffer)');
    }
    return this.gl.isRenderbuffer(renderbuffer ? renderbuffer._ : 0);
};

WebGLRenderingContext.prototype.isShader = function isShader(shader) {
    if (!(arguments.length === 1 && (shader === null || shader instanceof WebGLShader))) {
        throw new TypeError('Expected isShader(WebGLShader shader)');
    }
    return this.gl.isShader(shader ? shader._ : 0);
};

WebGLRenderingContext.prototype.isTexture = function isTexture(texture) {
    if (!(arguments.length === 1 && (texture === null || texture instanceof WebGLTexture))) {
        throw new TypeError('Expected isTexture(WebGLTexture texture)');
    }
    return this.gl.isTexture(texture ? texture._ : 0);
};

WebGLRenderingContext.prototype.lineWidth = function lineWidth(width) {
    if (!(arguments.length === 1 && typeof width === "number")) {
        throw new TypeError('Expected lineWidth(number width)');
    }
    return this.gl.lineWidth(width);
};

WebGLRenderingContext.prototype.linkProgram = function linkProgram(program) {
    if (!(arguments.length === 1 && (program === null || program instanceof WebGLProgram))) {
        throw new TypeError('Expected linkProgram(WebGLProgram program)');
    }
    return this.gl.linkProgram(program ? program._ : 0);
};

WebGLRenderingContext.prototype.pixelStorei = function pixelStorei(pname, param) {
    if (!(arguments.length === 2 && typeof pname === "number" && (typeof param === "number") || typeof param === "boolean")) {
        throw new TypeError('Expected pixelStorei(number pname, number param)');
    }

    if(typeof param === "boolean")
        param= param ? 1 : 0;
    return this.gl.pixelStorei(pname, param);
};

WebGLRenderingContext.prototype.polygonOffset = function polygonOffset(factor, units) {
    if (!(arguments.length === 2 && typeof factor === "number" && typeof units === "number")) {
        throw new TypeError('Expected polygonOffset(number factor, number units)');
    }
    return this.gl.polygonOffset(factor, units);
};

WebGLRenderingContext.prototype.readPixels = function readPixels(x, y, width, height, format, type, pixels) {
    if (!(arguments.length === 7 && typeof x === "number" && typeof y === "number" && typeof width === "number" && typeof height === "number" && typeof format === "number" && typeof type === "number" && typeof pixels === "object")) {
        throw new TypeError('Expected readPixels(number x, number y, number width, number height, number format, number type, ArrayBufferView pixels)');
    }
    return this.gl.readPixels(x, y, width, height, format, type, pixels);
};

WebGLRenderingContext.prototype.renderbufferStorage = function renderbufferStorage(target, internalformat, width, height) {
    if (!(arguments.length === 4 && typeof target === "number" && typeof internalformat === "number" && typeof width === "number" && typeof height === "number")) {
        throw new TypeError('Expected renderbufferStorage(number target, number internalformat, number width, number height)');
    }
    return this.gl.renderbufferStorage(target, internalformat, width, height);
};

WebGLRenderingContext.prototype.sampleCoverage = function sampleCoverage(value, invert) {
    if (!(arguments.length === 2 && typeof value === "number" && typeof invert === "boolean")) {
        throw new TypeError('Expected sampleCoverage(number value, boolean invert)');
    }
    return this.gl.sampleCoverage(value, invert);
};

WebGLRenderingContext.prototype.scissor = function scissor(x, y, width, height) {
    if (!(arguments.length === 4 && typeof x === "number" && typeof y === "number" && typeof width === "number" && typeof height === "number")) {
        throw new TypeError('Expected scissor(number x, number y, number width, number height)');
    }
    return this.gl.scissor(x, y, width, height);
};

WebGLRenderingContext.prototype.shaderSource = function shaderSource(shader, source) {
    if (!(arguments.length === 2 && (shader === null || shader instanceof WebGLShader) && typeof source === "string")) {
        throw new TypeError('Expected shaderSource(WebGLShader shader, string source)');
    }
    return this.gl.shaderSource(shader ? shader._ : 0, source);
};

WebGLRenderingContext.prototype.stencilFunc = function stencilFunc(func, ref, mask) {
    if (!(arguments.length === 3 && typeof func === "number" && typeof ref === "number" && typeof mask === "number")) {
        throw new TypeError('Expected stencilFunc(number func, number ref, number mask)');
    }
    return this.gl.stencilFunc(func, ref, mask);
};

WebGLRenderingContext.prototype.stencilFuncSeparate = function stencilFuncSeparate(face, func, ref, mask) {
    if (!(arguments.length === 4 && typeof face === "number" && typeof func === "number" && typeof ref === "number" && typeof mask === "number")) {
        throw new TypeError('Expected stencilFuncSeparate(number face, number func, number ref, number mask)');
    }
    return this.gl.stencilFuncSeparate(face, func, ref, mask);
};

WebGLRenderingContext.prototype.stencilMask = function stencilMask(mask) {
    if (!(arguments.length === 1 && typeof mask === "number")) {
        throw new TypeError('Expected stencilMask(number mask)');
    }
    return this.gl.stencilMask(mask);
};

WebGLRenderingContext.prototype.stencilMaskSeparate = function stencilMaskSeparate(face, mask) {
    if (!(arguments.length === 2 && typeof face === "number" && typeof mask === "number")) {
        throw new TypeError('Expected stencilMaskSeparate(number face, number mask)');
    }
    return this.gl.stencilMaskSeparate(face, mask);
};

WebGLRenderingContext.prototype.stencilOp = function stencilOp(fail, zfail, zpass) {
    if (!(arguments.length === 3 && typeof fail === "number" && typeof zfail === "number" && typeof zpass === "number")) {
        throw new TypeError('Expected stencilOp(number fail, number zfail, number zpass)');
    }
    return this.gl.stencilOp(fail, zfail, zpass);
};

WebGLRenderingContext.prototype.stencilOpSeparate = function stencilOpSeparate(face, fail, zfail, zpass) {
    if (!(arguments.length === 4 && typeof face === "number" && typeof fail === "number" && typeof zfail === "number" && typeof zpass === "number")) {
        throw new TypeError('Expected stencilOpSeparate(number face, number fail, number zfail, number zpass)');
    }
    return this.gl.stencilOpSeparate(face, fail, zfail, zpass);
};

WebGLRenderingContext.prototype.texImage2D = function texImage2D(target, level, internalformat, width, height, border, format, type, pixels) {
    if(!(arguments.length == 9)) {
        throw new TypeError('Expected texImage2D(number target, number level, number internalformat, number width, number height, number border, number format, number type, Buffer pixels)');
    }

    if(!(typeof target === "number" &&
        typeof level === "number" && typeof internalformat === "number" &&
        typeof width === "number" && typeof height === "number" && typeof border === "number" &&
        typeof format === "number" && typeof type === "number" &&
        (pixels === null || Buffer.isBuffer(pixels) || pixels instanceof Uint8Array))) {
        throw new TypeError('Expected texImage2D(number target, number level, number internalformat, number width, number height, number border, number format, number type, Buffer pixels)');
    }

    return this.gl.texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
};

WebGLRenderingContext.prototype.texParameterf = function texParameterf(target, pname, param) {
    if (!(arguments.length === 3 && typeof target === "number" && typeof pname === "number" && typeof param === "number")) {
        throw new TypeError('Expected texParameterf(number target, number pname, number param)');
    }
    return this.gl.texParameterf(target, pname, param);
};

WebGLRenderingContext.prototype.texParameteri = function texParameteri(target, pname, param) {
    if (!(arguments.length === 3 && typeof target === "number" && typeof pname === "number" && typeof param === "number")) {
        throw new TypeError('Expected texParameteri(number target, number pname, number param)');
    }
    return this.gl.texParameteri(target, pname, param);
};

WebGLRenderingContext.prototype.texSubImage2D = function texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels) {
    if (!(arguments.length === 9 && typeof target === "number" && typeof level === "number" &&
        typeof xoffset === "number" && typeof yoffset === "number" &&
        typeof width === "number" && typeof height === "number" &&
        typeof format === "number" && typeof type === "number" &&
        (pixels === null || Buffer.isBuffer(pixels) || pixels instanceof Uint8Array))) {
        throw new TypeError('Expected texSubImage2D(number target, number level, number xoffset, number yoffset, number width, number height, number format, number type, ArrayBufferView pixels)');
    }
    return this.gl.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
};

WebGLRenderingContext.prototype.uniform1f = function uniform1f(location, x) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && (typeof x === "number" || typeof x === "boolean"))) {
        throw new TypeError('Expected uniform1f(WebGLUniformLocation location, number x)');
    }
    return this.gl.uniform1f(location ? location._ : 0, x);
};

WebGLRenderingContext.prototype.uniform1fv = function uniform1fv(location, v) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && typeof v === "object")) {
        throw new TypeError('Expected uniform1fv(WebGLUniformLocation location, FloatArray v)');
    }
    return this.gl.uniform1fv(location ? location._ : 0, v);
};

WebGLRenderingContext.prototype.uniform1i = function uniform1i(location, x) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && (typeof x === "number" || typeof x ==="boolean"))) {
        throw new TypeError('Expected uniform1i(WebGLUniformLocation location, number x)');
    }
    if(typeof x === "boolean")
        x= x ? 1 : 0;
    return this.gl.uniform1i(location ? location._ : 0, x);
};

WebGLRenderingContext.prototype.uniform1iv = function uniform1iv(location, v) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && typeof v === "object")) {
        throw new TypeError('Expected uniform1iv(WebGLUniformLocation location, Int32Array v)');
    }
    return this.gl.uniform1iv(location ? location._ : 0, v);
};

WebGLRenderingContext.prototype.uniform2f = function uniform2f(location, x, y) {
    if (!(arguments.length === 3 && (location === null || location instanceof WebGLUniformLocation) && typeof x === "number" && typeof y === "number")) {
        throw new TypeError('Expected uniform2f(WebGLUniformLocation location, number x, number y)');
    }
    return this.gl.uniform2f(location ? location._ : 0, x, y);
};

WebGLRenderingContext.prototype.uniform2fv = function uniform2fv(location, v) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && typeof v === "object")) {
        throw new TypeError('Expected uniform2fv(WebGLUniformLocation location, FloatArray v)');
    }
    return this.gl.uniform2fv(location ? location._ : 0, v);
};

WebGLRenderingContext.prototype.uniform2i = function uniform2i(location, x, y) {
    if (!(arguments.length === 3 && (location === null || location instanceof WebGLUniformLocation) && typeof x === "number" && typeof y === "number")) {
        throw new TypeError('Expected uniform2i(WebGLUniformLocation location, number x, number y)');
    }
    return this.gl.uniform2i(location ? location._ : 0, x, y);
};

WebGLRenderingContext.prototype.uniform2iv = function uniform2iv(location, v) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && typeof v === "object")) {
        throw new TypeError('Expected uniform2iv(WebGLUniformLocation location, Int32Array v)');
    }
    return this.gl.uniform2iv(location ? location._ : 0, v);
};

WebGLRenderingContext.prototype.uniform3f = function uniform3f(location, x, y, z) {
    if (!(arguments.length === 4 && (location === null || location instanceof WebGLUniformLocation) && typeof x === "number" && typeof y === "number" && typeof z === "number")) {
        throw new TypeError('Expected uniform3f(WebGLUniformLocation location, number x, number y, number z)');
    }
    return this.gl.uniform3f(location ? location._ : 0, x, y, z);
};

WebGLRenderingContext.prototype.uniform3fv = function uniform3fv(location, v) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && typeof v === "object")) {
        throw new TypeError('Expected uniform3fv(WebGLUniformLocation location, FloatArray v)');
    }
    return this.gl.uniform3fv(location ? location._ : 0, v);
};

WebGLRenderingContext.prototype.uniform3i = function uniform3i(location, x, y, z) {
    if (!(arguments.length === 4 && (location === null || location instanceof WebGLUniformLocation) && typeof x === "number" && typeof y === "number" && typeof z === "number")) {
        throw new TypeError('Expected uniform3i(WebGLUniformLocation location, number x, number y, number z)');
    }
    return this.gl.uniform3i(location ? location._ : 0, x, y, z);
};

WebGLRenderingContext.prototype.uniform3iv = function uniform3iv(location, x) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && typeof x === "object")) {
        throw new TypeError('Expected uniform3iv(WebGLUniformLocation location, Int32Array x)');
    }
    return this.gl.uniform3iv(location ? location._ : 0, x);
};

WebGLRenderingContext.prototype.uniform4f = function uniform4f(location, x, y, z, w) {
    if (!(arguments.length === 5 && (location === null || location instanceof WebGLUniformLocation) && typeof x === "number" && typeof y === "number" && typeof z === "number" && typeof w === "number")) {
        throw new TypeError('Expected uniform4f(WebGLUniformLocation location, number x, number y, number z, number w)');
    }
    return this.gl.uniform4f(location ? location._ : 0, x, y, z, w);
};

WebGLRenderingContext.prototype.uniform4fv = function uniform4fv(location, v) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && typeof v === "object")) {
        throw new TypeError('Expected uniform4fv(WebGLUniformLocation location, FloatArray v)');
    }
    return this.gl.uniform4fv(location ? location._ : 0, v);
};

WebGLRenderingContext.prototype.uniform4i = function uniform4i(location, x, y, z, w) {
    if (!(arguments.length === 5 && (location === null || location instanceof WebGLUniformLocation) && typeof x === "number" && typeof y === "number" && typeof z === "number" && typeof w === "number")) {
        throw new TypeError('Expected uniform4i(WebGLUniformLocation location, number x, number y, number z, number w)');
    }
    return this.gl.uniform4i(location ? location._ : 0, x, y, z, w);
};

WebGLRenderingContext.prototype.uniform4iv = function uniform4iv(location, x) {
    if (!(arguments.length === 2 && (location === null || location instanceof WebGLUniformLocation) && typeof x === "object")) {
        throw new TypeError('Expected uniform4iv(WebGLUniformLocation location, Int32Array x)');
    }
    return this.gl.uniform4iv(location ? location._ : 0, x);
};

WebGLRenderingContext.prototype.uniformMatrix2fv = function uniformMatrix2fv(location, transpose, value) {
    if (!(arguments.length === 3 && (location === null || location instanceof WebGLUniformLocation) && typeof transpose === "boolean" && typeof value === "object")) {
        throw new TypeError('Expected uniformMatrix2fv(WebGLUniformLocation location, boolean transpose, FloatArray value)');
    }
    return this.gl.uniformMatrix2fv(location ? location._ : 0, transpose, value);
};

WebGLRenderingContext.prototype.uniformMatrix3fv = function uniformMatrix3fv(location, transpose, value) {
    if (!(arguments.length === 3 && (location === null || location instanceof WebGLUniformLocation) && typeof transpose === "boolean" && typeof value === "object")) {
        throw new TypeError('Expected uniformMatrix3fv(WebGLUniformLocation location, boolean transpose, FloatArray value)');
    }
    return this.gl.uniformMatrix3fv(location ? location._ : 0, transpose, value);
};

WebGLRenderingContext.prototype.uniformMatrix4fv = function uniformMatrix4fv(location, transpose, value) {
    if (!(arguments.length === 3 && (location === null || location instanceof WebGLUniformLocation) && typeof transpose === "boolean" && typeof value === "object")) {
        throw new TypeError('Expected uniformMatrix4fv(WebGLUniformLocation location, boolean transpose, FloatArray value)');
    }
    return this.gl.uniformMatrix4fv(location ? location._ : 0, transpose, value);
};

WebGLRenderingContext.prototype.useProgram = function useProgram(program) {
    if (!(arguments.length === 1 && (program === null || program instanceof WebGLProgram))) {
        throw new TypeError('Expected useProgram(WebGLProgram program)');
    }
    return this.gl.useProgram(program ? program._ : 0);
};

WebGLRenderingContext.prototype.validateProgram = function validateProgram(program) {
    if (!(arguments.length === 1 && (program === null || program instanceof WebGLProgram))) {
        throw new TypeError('Expected validateProgram(WebGLProgram program)');
    }
    return this.gl.validateProgram(program ? program._ : 0);
};

WebGLRenderingContext.prototype.vertexAttrib1f = function vertexAttrib1f(indx, x) {
    if (!(arguments.length === 2 && typeof indx === "number" && typeof x === "number")) {
        throw new TypeError('Expected vertexAttrib1f(number indx, number x)');
    }
    return this.gl.vertexAttrib1f(indx, x);
};

WebGLRenderingContext.prototype.vertexAttrib1fv = function vertexAttrib1fv(indx, values) {
    if (!(arguments.length === 2 && typeof indx === "number" && typeof values === "object")) {
        throw new TypeError('Expected vertexAttrib1fv(number indx, FloatArray values)');
    }
    return this.gl.vertexAttrib1fv(indx, values);
};

WebGLRenderingContext.prototype.vertexAttrib2f = function vertexAttrib2f(indx, x, y) {
    if (!(arguments.length === 3 && typeof indx === "number" && typeof x === "number" && typeof y === "number")) {
        throw new TypeError('Expected vertexAttrib2f(number indx, number x, number y)');
    }
    return this.gl.vertexAttrib2f(indx, x, y);
};

WebGLRenderingContext.prototype.vertexAttrib2fv = function vertexAttrib2fv(indx, values) {
    if (!(arguments.length === 2 && typeof indx === "number" && typeof values === "object")) {
        throw new TypeError('Expected vertexAttrib2fv(number indx, FloatArray values)');
    }
    return this.gl.vertexAttrib2fv(indx, values);
};

WebGLRenderingContext.prototype.vertexAttrib3f = function vertexAttrib3f(indx, x, y, z) {
    if (!(arguments.length === 4 && typeof indx === "number" && typeof x === "number" && typeof y === "number" && typeof z === "number")) {
        throw new TypeError('Expected vertexAttrib3f(number indx, number x, number y, number z)');
    }
    return this.gl.vertexAttrib3f(indx, x, y, z);
};

WebGLRenderingContext.prototype.vertexAttrib3fv = function vertexAttrib3fv(indx, values) {
    if (!(arguments.length === 2 && typeof indx === "number" && typeof values === "object")) {
        throw new TypeError('Expected vertexAttrib3fv(number indx, FloatArray values)');
    }
    return this.gl.vertexAttrib3fv(indx, values);
};

WebGLRenderingContext.prototype.vertexAttrib4f = function vertexAttrib4f(indx, x, y, z, w) {
    if (!(arguments.length === 5 && typeof indx === "number" && typeof x === "number" && typeof y === "number" && typeof z === "number" && typeof w === "number")) {
        throw new TypeError('Expected vertexAttrib4f(number indx, number x, number y, number z, number w)');
    }
    return this.gl.vertexAttrib4f(indx, x, y, z, w);
};

WebGLRenderingContext.prototype.vertexAttrib4fv = function vertexAttrib4fv(indx, values) {
    if (!(arguments.length === 2 && typeof indx === "number" && typeof values === "object")) {
        throw new TypeError('Expected vertexAttrib4fv(number indx, FloatArray values)');
    }
    return this.gl.vertexAttrib4fv(indx, values);
};

WebGLRenderingContext.prototype.vertexAttribPointer = function vertexAttribPointer(indx, size, type, normalized, stride, offset) {
    if (!(arguments.length === 6 && typeof indx === "number" && typeof size === "number" && typeof type === "number" && (typeof normalized === "boolean" || typeof normalized === "number") && typeof stride === "number" && typeof offset === "number")) {
        throw new TypeError('Expected vertexAttribPointer(number indx, number size, number type, boolean normalized, number stride, number offset)');
    }
    return this.gl.vertexAttribPointer(indx, size, type, normalized, stride, offset);
};

WebGLRenderingContext.prototype.viewport = function viewport(x, y, width, height) {
    if (!(arguments.length === 4 && typeof x === "number" && typeof y === "number" && typeof width === "number" && typeof height === "number")) {
        throw new TypeError('Expected viewport(number x, number y, number width, number height)');
    }
    return this.gl.viewport(x, y, width, height);
};
