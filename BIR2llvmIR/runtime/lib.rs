// Rust Library

// String comparision
#[no_mangle]
pub extern "C" fn is_same_type(src_type : &str, dest_type : &str) -> bool
{
  return src_type == dest_type;;
}

// Returns string object based on passed string literal
#[no_mangle]
pub extern "C" fn str_to_strobj(cstring : &str) -> std::string::String
{
  println!("{}", cstring);
  let content_string = String::from(cstring);
  return content_string;
}

// Prints 64 bit signed integer
#[no_mangle]
pub extern "C" fn print64(num64 : i64)
{
   println!("{}", num64);
}

// Prints 32 bit signed integer
#[no_mangle]
pub extern "C" fn print32(num32 : i32)
{
   println!("{}", num32);
}

// Prints 16 bit signed integer
#[no_mangle]
pub extern "C" fn print16(num16 : i16)
{
   println!("{}", num16);
}

// Prints 8 bit signed integer
#[no_mangle]
pub extern "C" fn print8(num8 : i8)
{
   println!("{}", num8);
}

// Prints 64 bit unsigned integer
#[no_mangle]
pub extern "C" fn printu64(num64 : u64)
{
   println!("{}", num64);
}

// Prints 32 bit unsigned integer
#[no_mangle]
pub extern "C" fn printu32(num32 : u32)
{
   println!("{}", num32);
}

// Prints 16 bit unsigned integer
#[no_mangle]
pub extern "C" fn printu16(num16 : u16)
{
   println!("{}", num16);
}

// Prints 8 bit unsigned integer
#[no_mangle]
pub extern "C" fn printu8(num8 : u8)
{
   println!("{}", num8);
}

// Prints 64 bit float
#[no_mangle]
pub extern "C" fn printf64(num64 : f64)
{
   println!("{}", num64);
}

// Prints 32 bit float
#[no_mangle]
pub extern "C" fn printf32(num32 : f32)
{
   println!("{}", num32);
}

// Prints string
#[no_mangle]
pub extern "C" fn print_str(string : &str)
{
   println!("{}", string);
}
