; ModuleID = 'CminorException.cpp'
source_filename = "CminorException.cpp"
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.10.25017"

%rtti.TypeDescriptor36 = type { i8**, i8*, [37 x i8] }
%"class.cminor::util::StringPtr" = type { i32* }
%"class.cminor::machine::CminorException" = type { i64 }
%"class.cminor::machine::ClassData" = type { %"class.cminor::machine::ObjectType"*, %"class.cminor::machine::MethodTable", %"class.std::vector.8", %"class.std::unique_ptr" }
%"class.cminor::machine::ObjectType" = type { %"class.cminor::machine::Type", i64, %"class.cminor::machine::Layout" }
%"class.cminor::machine::Type" = type { i32 (...)**, %"class.cminor::machine::Constant" }
%"class.cminor::machine::Constant" = type { %"class.cminor::machine::IntegralValue" }
%"class.cminor::machine::IntegralValue" = type { %union.anon, i8 }
%union.anon = type { i64 }
%"class.cminor::machine::Layout" = type { %"class.std::vector", i64 }
%"class.std::vector" = type { %"class.std::_Vector_alloc" }
%"class.std::_Vector_alloc" = type { %"class.std::_Compressed_pair" }
%"class.std::_Compressed_pair" = type { %"class.std::_Vector_val" }
%"class.std::_Vector_val" = type { %"class.cminor::machine::Field"*, %"class.cminor::machine::Field"*, %"class.cminor::machine::Field"* }
%"class.cminor::machine::Field" = type { i8, %"class.cminor::machine::FieldOffset" }
%"class.cminor::machine::FieldOffset" = type { i32 }
%"class.cminor::machine::MethodTable" = type { %"class.std::vector.0" }
%"class.std::vector.0" = type { %"class.std::_Vector_alloc.1" }
%"class.std::_Vector_alloc.1" = type { %"class.std::_Compressed_pair.2" }
%"class.std::_Compressed_pair.2" = type { %"class.std::_Vector_val.7" }
%"class.std::_Vector_val.7" = type { %"class.cminor::machine::Constant"*, %"class.cminor::machine::Constant"*, %"class.cminor::machine::Constant"* }
%"class.std::vector.8" = type { %"class.std::_Vector_alloc.9" }
%"class.std::_Vector_alloc.9" = type { %"class.std::_Compressed_pair.10" }
%"class.std::_Compressed_pair.10" = type { %"class.std::_Vector_val.15" }
%"class.std::_Vector_val.15" = type { %"class.cminor::machine::MethodTable"*, %"class.cminor::machine::MethodTable"*, %"class.cminor::machine::MethodTable"* }
%"class.std::unique_ptr" = type { %"class.std::_Unique_ptr_base" }
%"class.std::_Unique_ptr_base" = type { %"class.std::_Compressed_pair.16" }
%"class.std::_Compressed_pair.16" = type { %"class.cminor::machine::StaticClassData"* }
%"class.cminor::machine::StaticClassData" = type { %"struct.std::atomic", %"struct.std::atomic", %"class.std::recursive_mutex", %"class.cminor::machine::Constant", %"class.cminor::machine::Layout", i8* }
%"struct.std::atomic" = type { %"struct.std::_Atomic_bool" }
%"struct.std::_Atomic_bool" = type { i8 }
%"class.std::recursive_mutex" = type { %"class.std::_Mutex_base" }
%"class.std::_Mutex_base" = type { %"union.std::_Align_type" }
%"union.std::_Align_type" = type { double, [72 x i8] }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

$"\01??0StringPtr@util@cminor@@QEAA@PEB_U@Z" = comdat any

$"\01??_R0?AVCminorException@machine@cminor@@@8" = comdat any

$"\01??_C@_0EE@MMFLPACK@S?$AA?$AA?$AAy?$AA?$AA?$AAs?$AA?$AA?$AAt?$AA?$AA?$AAe?$AA?$AA?$AAm?$AA?$AA?$AA?4?$AA?$AA?$AAE?$AA?$AA?$AAx?$AA?$AA?$AAc?$AA?$AA?$AAe?$AA?$AA?$AAp?$AA?$AA?$AAt?$AA?$AA?$AAi?$AA?$AA?$AAo?$AA?$AA?$AAn?$AA?$AA?$AA?$AA?$AA?$AA?$AA@" = comdat any

@"\01?cd@machine@cminor@@3PEAXEA" = global i8* null, align 8
@"\01??_7type_info@@6B@" = external constant i8*
@"\01??_R0?AVCminorException@machine@cminor@@@8" = linkonce_odr global %rtti.TypeDescriptor36 { i8** @"\01??_7type_info@@6B@", i8* null, [37 x i8] c".?AVCminorException@machine@cminor@@\00" }, comdat
@"\01??_C@_0EE@MMFLPACK@S?$AA?$AA?$AAy?$AA?$AA?$AAs?$AA?$AA?$AAt?$AA?$AA?$AAe?$AA?$AA?$AAm?$AA?$AA?$AA?4?$AA?$AA?$AAE?$AA?$AA?$AAx?$AA?$AA?$AAc?$AA?$AA?$AAe?$AA?$AA?$AAp?$AA?$AA?$AAt?$AA?$AA?$AAi?$AA?$AA?$AAo?$AA?$AA?$AAn?$AA?$AA?$AA?$AA?$AA?$AA?$AA@" = linkonce_odr unnamed_addr constant [17 x i32] [i32 83, i32 121, i32 115, i32 116, i32 101, i32 109, i32 46, i32 69, i32 120, i32 99, i32 101, i32 112, i32 116, i32 105, i32 111, i32 110, i32 0], comdat, align 4

; Function Attrs: noinline uwtable
define void @"\01?foo@machine@cminor@@YAXXZ"() #0 {
entry:
  %o = alloca i64, align 8
  %0 = load i8*, i8** @"\01?cd@machine@cminor@@3PEAXEA", align 8
  %call = call i64 @RtCreateObject(i8* %0)
  store i64 %call, i64* %o, align 8
  %1 = load i8*, i8** @"\01?cd@machine@cminor@@3PEAXEA", align 8
  %2 = load i64, i64* %o, align 8
  call void @RtSetClassDataPtr(i64 %2, i8* %1)
  %3 = load i64, i64* %o, align 8
  call void @RtThrowException(i64 %3)
  ret void
}

declare dllimport i64 @RtCreateObject(i8*) #1

declare dllimport void @RtSetClassDataPtr(i64, i8*) #1

declare dllimport void @RtThrowException(i64) #1

; Function Attrs: noinline uwtable
define void @"\01?ThrowCatch@machine@cminor@@YAXXZ"() #0 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %agg.tmp = alloca %"class.cminor::util::StringPtr", align 8
  %ex = alloca %"class.cminor::machine::CminorException"*, align 8
  %x = alloca i64, align 8
  %call = invoke %"class.cminor::util::StringPtr"* @"\01??0StringPtr@util@cminor@@QEAA@PEB_U@Z"(%"class.cminor::util::StringPtr"* %agg.tmp, i32* getelementptr inbounds ([17 x i32], [17 x i32]* @"\01??_C@_0EE@MMFLPACK@S?$AA?$AA?$AAy?$AA?$AA?$AAs?$AA?$AA?$AAt?$AA?$AA?$AAe?$AA?$AA?$AAm?$AA?$AA?$AA?4?$AA?$AA?$AAE?$AA?$AA?$AAx?$AA?$AA?$AAc?$AA?$AA?$AAe?$AA?$AA?$AAp?$AA?$AA?$AAt?$AA?$AA?$AAi?$AA?$AA?$AAo?$AA?$AA?$AAn?$AA?$AA?$AA?$AA?$AA?$AA?$AA@", i32 0, i32 0))
          to label %invoke.cont unwind label %catch.dispatch

invoke.cont:                                      ; preds = %entry
  %coerce.dive = getelementptr inbounds %"class.cminor::util::StringPtr", %"class.cminor::util::StringPtr"* %agg.tmp, i32 0, i32 0
  %0 = load i32*, i32** %coerce.dive, align 8
  %coerce.val.pi = ptrtoint i32* %0 to i64
  %call2 = invoke %"class.cminor::machine::ClassData"* @"\01?GetClassData@ClassDataTable@machine@cminor@@SAPEAVClassData@23@VStringPtr@util@3@@Z"(i64 %coerce.val.pi)
          to label %invoke.cont1 unwind label %catch.dispatch

invoke.cont1:                                     ; preds = %invoke.cont
  %1 = bitcast %"class.cminor::machine::ClassData"* %call2 to i8*
  store i8* %1, i8** @"\01?cd@machine@cminor@@3PEAXEA", align 8
  invoke void @"\01?foo@machine@cminor@@YAXXZ"()
          to label %invoke.cont3 unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %invoke.cont1, %invoke.cont, %entry
  %2 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %3 = catchpad within %2 [%rtti.TypeDescriptor36* @"\01??_R0?AVCminorException@machine@cminor@@@8", i32 8, %"class.cminor::machine::CminorException"** %ex]
  %4 = load %"class.cminor::machine::CminorException"*, %"class.cminor::machine::CminorException"** %ex, align 8
  %call4 = call i64 @"\01?ExceptionObjectReference@CminorException@machine@cminor@@QEBA_KXZ"(%"class.cminor::machine::CminorException"* %4) [ "funclet"(token %3) ]
  store i64 %call4, i64* %x, align 8
  %5 = load i8*, i8** @"\01?cd@machine@cminor@@3PEAXEA", align 8
  %call5 = call zeroext i1 @RtHandleException(i8* %5) [ "funclet"(token %3) ]
  br i1 %call5, label %if.then, label %if.end

if.then:                                          ; preds = %catch
  br label %if.end

invoke.cont3:                                     ; preds = %invoke.cont1
  br label %try.cont

if.end:                                           ; preds = %if.then, %catch
  catchret from %3 to label %catchret.dest

catchret.dest:                                    ; preds = %if.end
  br label %try.cont

try.cont:                                         ; preds = %catchret.dest, %invoke.cont3
  ret void
}

declare dllimport %"class.cminor::machine::ClassData"* @"\01?GetClassData@ClassDataTable@machine@cminor@@SAPEAVClassData@23@VStringPtr@util@3@@Z"(i64) #1

; Function Attrs: noinline nounwind uwtable
define linkonce_odr %"class.cminor::util::StringPtr"* @"\01??0StringPtr@util@cminor@@QEAA@PEB_U@Z"(%"class.cminor::util::StringPtr"* returned %this, i32* %value_) unnamed_addr #2 comdat align 2 {
entry:
  %value_.addr = alloca i32*, align 8
  %this.addr = alloca %"class.cminor::util::StringPtr"*, align 8
  store i32* %value_, i32** %value_.addr, align 8
  store %"class.cminor::util::StringPtr"* %this, %"class.cminor::util::StringPtr"** %this.addr, align 8
  %this1 = load %"class.cminor::util::StringPtr"*, %"class.cminor::util::StringPtr"** %this.addr, align 8
  %value = getelementptr inbounds %"class.cminor::util::StringPtr", %"class.cminor::util::StringPtr"* %this1, i32 0, i32 0
  %0 = load i32*, i32** %value_.addr, align 8
  store i32* %0, i32** %value, align 8
  ret %"class.cminor::util::StringPtr"* %this1
}

declare i32 @__CxxFrameHandler3(...)

declare dllimport i64 @"\01?ExceptionObjectReference@CminorException@machine@cminor@@QEBA_KXZ"(%"class.cminor::machine::CminorException"*) #1

declare dllimport zeroext i1 @RtHandleException(i8*) #1

; Function Attrs: noinline uwtable
define void @"\01?CatchAll@machine@cminor@@YAXXZ"() #0 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  invoke void @"\01?foo@machine@cminor@@YAXXZ"()
          to label %invoke.cont unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %entry
  %0 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %1 = catchpad within %0 [i8* null, i32 64, i8* null]
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) #3 [ "funclet"(token %1) ]
  unreachable

invoke.cont:                                      ; preds = %entry
  br label %try.cont

try.cont:                                         ; preds = %invoke.cont
  ret void
}

declare dllimport void @_CxxThrowException(i8*, %eh.ThrowInfo*)

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn }

!llvm.module.flags = !{!0, !7}
!llvm.ident = !{!8}

!0 = !{i32 6, !"Linker Options", !1}
!1 = !{!2, !3, !4, !5, !6}
!2 = !{!"/FAILIFMISMATCH:\22_MSC_VER=1900\22"}
!3 = !{!"/FAILIFMISMATCH:\22_ITERATOR_DEBUG_LEVEL=0\22"}
!4 = !{!"/FAILIFMISMATCH:\22RuntimeLibrary=MT_StaticRelease\22"}
!5 = !{!"/DEFAULTLIB:libcpmt.lib"}
!6 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!7 = !{i32 1, !"PIC Level", i32 2}
!8 = !{!"clang version 4.0.0 (tags/RELEASE_400/final)"}
