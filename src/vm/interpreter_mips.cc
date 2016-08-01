// Copyright (c) 2016, the Dartino project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

#if defined(DARTINO_TARGET_MIPS)

#include "src/shared/bytecodes.h"
#include "src/shared/names.h"
#include "src/shared/selectors.h"

#include "src/vm/assembler.h"
#include "src/vm/generator.h"
#include "src/vm/interpreter.h"
#include "src/vm/intrinsics.h"
#include "src/vm/object.h"
#include "src/vm/process.h"
#include "src/vm/program.h"

#define __ assembler()->

namespace dartino {

class InterpreterGenerator {
 public:
  explicit InterpreterGenerator(Assembler* assembler)
      : assembler_(assembler) { }

  void Generate();

  virtual void GeneratePrologue() = 0;
  virtual void GenerateEpilogue() = 0;

  virtual void GenerateMethodEntry() = 0;

  virtual void GenerateBytecodePrologue(const char* name) = 0;
  virtual void GenerateDebugAtBytecode() = 0;

#define V(name, branching, format, size, stack_diff, print) \
  virtual void Do##name() = 0;
  BYTECODES_DO(V)
#undef V

#define V(name) virtual void DoIntrinsic##name() = 0;
  INTRINSICS_DO(V)
#undef V

 protected:
  Assembler* assembler() const { return assembler_; }

 private:
  Assembler* const assembler_;
};

void InterpreterGenerator::Generate() {
  GeneratePrologue();
  GenerateEpilogue();

  GenerateMethodEntry();

  GenerateDebugAtBytecode();

#define V(name, branching, format, size, stack_diff, print) \
  GenerateBytecodePrologue("BC_" #name);                    \
  Do##name();
  BYTECODES_DO(V)
#undef V

#define V(name)           \
  __ AlignToPowerOfTwo(3);  \
  __ Bind("", "Intrinsic_" #name); \
  DoIntrinsic##name();
  INTRINSICS_DO(V)
#undef V

#define V(name, branching, format, size, stack_diff, print) \
  assembler()->DefineLong("BC_" #name);
  BYTECODES_DO(V)
#undef V

  __ SwitchToData();
  __ BindWithPowerOfTwoAlignment("Interpret_DispatchTable", 4);
#define V(name, branching, format, size, stack_diff, print) \
  assembler()->DefineLong("BC_" #name);
  BYTECODES_DO(V)
#undef V
}

class InterpreterGeneratorMIPS: public InterpreterGenerator {
 public:
  explicit InterpreterGeneratorMIPS(Assembler* assembler)
      : InterpreterGenerator(assembler) { }

  // Registers
  // ---------
  // S0: current process
  // S1: bytecode pointer
  // S2: stack pointer (top)
  // S4: null
  // S6: program
  // S7: dispatch table
  virtual void GeneratePrologue();
  virtual void GenerateEpilogue();

  virtual void GenerateMethodEntry();

  virtual void GenerateBytecodePrologue(const char* name);
  virtual void GenerateDebugAtBytecode();

  virtual void DoLoadLocal0();
  virtual void DoLoadLocal1();
  virtual void DoLoadLocal2();
  virtual void DoLoadLocal3();
  virtual void DoLoadLocal4();
  virtual void DoLoadLocal5();
  virtual void DoLoadLocal();
  virtual void DoLoadLocalWide();

  virtual void DoLoadBoxed();
  virtual void DoLoadStatic();
  virtual void DoLoadStaticInit();
  virtual void DoLoadField();
  virtual void DoLoadFieldWide();

  virtual void DoLoadConst();

  virtual void DoStoreLocal();
  virtual void DoStoreBoxed();
  virtual void DoStoreStatic();
  virtual void DoStoreField();
  virtual void DoStoreFieldWide();

  virtual void DoLoadLiteralNull();
  virtual void DoLoadLiteralTrue();
  virtual void DoLoadLiteralFalse();
  virtual void DoLoadLiteral0();
  virtual void DoLoadLiteral1();
  virtual void DoLoadLiteral();
  virtual void DoLoadLiteralWide();

  virtual void DoInvokeMethodUnfold();
  virtual void DoInvokeMethod();

  virtual void DoInvokeNoSuchMethod();
  virtual void DoInvokeTestNoSuchMethod();

  virtual void DoInvokeStatic();
  virtual void DoInvokeFactory();

  virtual void DoInvokeLeafNative();
  virtual void DoInvokeNative();
  virtual void DoInvokeNativeYield();

  virtual void DoInvokeTestUnfold();
  virtual void DoInvokeTest();

  virtual void DoInvokeSelector();

#define INVOKE_BUILTIN(kind)               \
  virtual void DoInvoke##kind##Unfold() {  \
    Invoke##kind("BC_InvokeMethodUnfold"); \
  }                                        \
  virtual void DoInvoke##kind() { Invoke##kind("BC_InvokeMethod"); }

  INVOKE_BUILTIN(Eq);
  INVOKE_BUILTIN(Lt);
  INVOKE_BUILTIN(Le);
  INVOKE_BUILTIN(Gt);
  INVOKE_BUILTIN(Ge);

  INVOKE_BUILTIN(Add);
  INVOKE_BUILTIN(Sub);
  INVOKE_BUILTIN(Mod);
  INVOKE_BUILTIN(Mul);
  INVOKE_BUILTIN(TruncDiv);

  INVOKE_BUILTIN(BitNot);
  INVOKE_BUILTIN(BitAnd);
  INVOKE_BUILTIN(BitOr);
  INVOKE_BUILTIN(BitXor);
  INVOKE_BUILTIN(BitShr);
  INVOKE_BUILTIN(BitShl);

#undef INVOKE_BUILTIN

  virtual void DoPop();
  virtual void DoDrop();
  virtual void DoReturn();
  virtual void DoReturnNull();

  virtual void DoBranchWide();
  virtual void DoBranchIfTrueWide();
  virtual void DoBranchIfFalseWide();

  virtual void DoBranchBack();
  virtual void DoBranchBackIfTrue();
  virtual void DoBranchBackIfFalse();

  virtual void DoBranchBackWide();
  virtual void DoBranchBackIfTrueWide();
  virtual void DoBranchBackIfFalseWide();

  virtual void DoPopAndBranchWide();
  virtual void DoPopAndBranchBackWide();

  virtual void DoAllocate();
  virtual void DoAllocateImmutable();
  virtual void DoAllocateBoxed();

  virtual void DoNegate();

  virtual void DoStackOverflowCheck();

  virtual void DoThrow();
  virtual void DoThrowAfterSaveState(Label* resume);
  virtual void DoSubroutineCall();
  virtual void DoSubroutineReturn();

  virtual void DoProcessYield();
  virtual void DoCoroutineChange();

  virtual void DoIdentical();
  virtual void DoIdenticalNonNumeric();

  virtual void DoEnterNoSuchMethod();
  virtual void DoExitNoSuchMethod();

  virtual void DoMethodEnd();

  virtual void DoIntrinsicObjectEquals();
  virtual void DoIntrinsicGetField();
  virtual void DoIntrinsicSetField();
  virtual void DoIntrinsicListIndexGet();
  virtual void DoIntrinsicListIndexSet();
  virtual void DoIntrinsicListLength();

 private:
  Label done_;
  Label done_state_saved_;
  Label check_stack_overflow_;
  Label check_stack_overflow_0_;
  Label gc_;
  Label intrinsic_failure_;
  Label interpreter_entry_;
  int spill_size_;
  // Used in GeneratePrologue/Epilogue, S0-S7 + RA + FP.
  static const int kSaveFrameSize = 10;
  static const int kFalseOffset = 8;
  static const int kTrueOffset = 16;

  void LoadLocal(Register reg, int index);
  void StoreLocal(Register reg, int index);

  void Push(Register reg);
  void Pop(Register reg);
  void Drop(int n);
  void Drop(Register reg);
  void DropNAndSetTop(int dropping_slots, Register reg);

  void LoadFramePointer(Register reg);
  void StoreFramePointer(Register reg);

  void SaveByteCodePointer(Register scratch);
  void RestoreByteCodePointer(Register scratch);

  void PushFrameDescriptor(Register return_address, Register scratch);
  void ReadFrameDescriptor(Register scratch);

  void Return(bool is_return_null);

  void Allocate(bool immutable);

  // This function trashes 'scratch'.
  void AddToRememberedSet(Register object, Register value, Register scratch);

  void InvokeEq(const char* fallback);
  void InvokeLt(const char* fallback);
  void InvokeLe(const char* fallback);
  void InvokeGt(const char* fallback);
  void InvokeGe(const char* fallback);
  void InvokeCompare(const char* fallback, Condition condition);

  void InvokeAdd(const char* fallback);
  void InvokeSub(const char* fallback);
  void InvokeMod(const char* fallback);
  void InvokeMul(const char* fallback);
  void InvokeTruncDiv(const char* fallback);

  void InvokeBitNot(const char* fallback);
  void InvokeBitAnd(const char* fallback);
  void InvokeBitOr(const char* fallback);
  void InvokeBitXor(const char* fallback);
  void InvokeBitShr(const char* fallback);
  void InvokeBitShl(const char* fallback);

  void InvokeMethodUnfold(bool test);
  void InvokeMethod(bool test);

  void InvokeNative(bool yield, bool safepoint);
  void InvokeStatic();

  void AddIf(Register reg1, Register reg2, Condition cond,
             Register scratch1, Register scratch2,
             Register reg, int value);
  void ToBoolean(Register reg1, Register reg2, Condition cond, Register reg);
  void LoadFalse(Register reg);
  void LoadTrue(Register reg);

  void CheckStackOverflow(int size);

  void Dispatch(int size);

  void SaveState(Label* resume);
  void RestoreState();

  void ShiftAddJump(Register reg1, Register reg2, int imm);
  void ShiftAddLoad(Register reg1, Register reg2, Register reg3, int imm);
  void ShiftAddStore(Register reg1, Register reg2, Register reg3, int imm);
  void ShiftAdd(Register reg1, Register reg2, Register reg3, int imm);
  void ShiftSub(Register reg1, Register reg2, Register reg3, int imm);
  void ShiftRightAdd(Register reg1, Register reg2, Register reg3, int imm);
  void PrepareStack();
  void RestoreStack();

  static int ComputeStackPadding(int reserved, int extra) {
    const int kAlignment = 8;
    int rounded = (reserved + extra + kAlignment - 1) & ~(kAlignment - 1);
    return rounded - reserved;
  }
};

GENERATE(, Interpret) {
  InterpreterGeneratorMIPS generator(assembler);
  generator.Generate();
}

void InterpreterGeneratorMIPS::GeneratePrologue() {
  // Push callee-saved registers and reserve one extra slot.
  __ addiu(SP, SP, Immediate(-kSaveFrameSize * kWordSize));
  __ sw(S0, Address(SP, 9 * kWordSize));
  __ sw(S1, Address(SP, 8 * kWordSize));
  __ sw(S2, Address(SP, 7 * kWordSize));
  __ sw(S3, Address(SP, 6 * kWordSize));
  __ sw(S4, Address(SP, 5 * kWordSize));
  __ sw(S5, Address(SP, 4 * kWordSize));
  __ sw(S6, Address(SP, 3 * kWordSize));
  __ sw(S7, Address(SP, 2 * kWordSize));
  __ sw(RA, Address(SP, 1 * kWordSize));

  // Setup process pointer in S0
  __ move(S0, A0);

  // Pad the stack to guarantee the right alignment for calls.
  spill_size_ = ComputeStackPadding(kSaveFrameSize * kWordSize, 1 * kWordSize);
  if (spill_size_ > 0) __ addiu(SP, SP, Immediate(-1 * spill_size_));

  // Store the argument target yield address in the extra slot on the
  // top of the stack.
  __ sw(A1, Address(SP, 0));

  // Restore the register state and dispatch to the first bytecode.
  RestoreState();
}

void InterpreterGeneratorMIPS::GenerateEpilogue() {
  // Done. Save the register state.
  __ Bind(&done_);
  SaveState(&interpreter_entry_);

  // Undo stack padding.
  __ Bind(&done_state_saved_);
  if (spill_size_ > 0) __ addiu(SP, SP, Immediate(spill_size_));

  // Restore callee-saved registers and return.
  __ lw(RA, Address(SP, 1 * kWordSize));
  __ lw(S7, Address(SP, 2 * kWordSize));
  __ lw(S6, Address(SP, 3 * kWordSize));
  __ lw(S5, Address(SP, 4 * kWordSize));
  __ lw(S4, Address(SP, 5 * kWordSize));
  __ lw(S3, Address(SP, 6 * kWordSize));
  __ lw(S2, Address(SP, 7 * kWordSize));
  __ lw(S1, Address(SP, 8 * kWordSize));
  __ lw(S0, Address(SP, 9 * kWordSize));

  __ jr(RA);
  __ addiu(SP, SP, Immediate(kSaveFrameSize * kWordSize));  // Delay-slot.

  // Default entrypoint.
  __ Bind("", "InterpreterEntry");
  __ Bind(&interpreter_entry_);
  Dispatch(0);

  // Handle GC and re-interpret current bytecode.
  __ Bind(&gc_);
  SaveState(&interpreter_entry_);
  PrepareStack();
  __ la(T9, "HandleGC");
  __ jalr(T9);
  __ move(A0, S0);  // Delay-slot.
  RestoreStack();
  RestoreState();

  // Stack overflow handling (slow case).
  Label stay_fast, overflow, check_debug_interrupt, overflow_resume;
  __ Bind(&check_stack_overflow_0_);
  __ move(A0, ZR);
  __ Bind(&check_stack_overflow_);
  SaveState(&overflow_resume);

  __ move(A1, A0);
  PrepareStack();
  __ la(T9, "HandleStackOverflow");
  __ jalr(T9);
  __ move(A0, S0);  // Delay-slot.
  RestoreStack();
  RestoreState();
  __ Bind(&overflow_resume);
  ASSERT(Process::kStackCheckContinue == 0);
  __ B(EQ, V0, ZR, &stay_fast);
  __ li(T0, Immediate(Process::kStackCheckInterrupt));
  __ B(NEQ, V0, T0, &check_debug_interrupt);
  __ b(&done_);
  __ ori(V0, ZR, Immediate(Interpreter::kInterrupt));  // Delay-slot.
  __ Bind(&check_debug_interrupt);
  __ li(T0, Immediate(Process::kStackCheckDebugInterrupt));
  __ B(NEQ, V0, T0, &overflow);
  __ b(&done_);
  __ ori(V0, ZR, Immediate(Interpreter::kBreakpoint));  // Delay-slot.

  __ Bind(&stay_fast);
  Dispatch(0);

  __ Bind(&overflow);
  Label throw_resume;
  SaveState(&throw_resume);
  __ lw(S3, Address(S6, Program::kStackOverflowErrorOffset));
  DoThrowAfterSaveState(&throw_resume);

  // Intrinsic failure: Just invoke the method.
  __ Bind(&intrinsic_failure_);
  __ la(T9, "InterpreterMethodEntry");
  __ Jr(T9);
}

void InterpreterGeneratorMIPS::GenerateMethodEntry() {
  __ SwitchToText();
  __ AlignToPowerOfTwo(3);
  __ Bind("", "InterpreterMethodEntry");
  Push(RA);
  LoadFramePointer(A2);
  Push(A2);
  StoreFramePointer(S2);
  Push(ZR);
  __ addiu(S1, A0, Immediate(Function::kSize - HeapObject::kTag));
  CheckStackOverflow(0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::GenerateBytecodePrologue(const char* name) {
  __ SwitchToText();
  __ AlignToPowerOfTwo(3);
  __ nop();
  __ Bind("Debug_", name);
  __ la(T9, "DebugAtBytecode");
  __ Jalr(T9);
  __ AlignToPowerOfTwo(3);
  __ Bind("", name);
}

void InterpreterGeneratorMIPS::GenerateDebugAtBytecode() {
  __ SwitchToText();
  __ AlignToPowerOfTwo(4);  // Align to 8-byte storage boundary.
  __ Bind("", "DebugAtBytecode");
  __ move(S3, RA);
  __ move(A0, S0);
  __ move(A1, S1);
  PrepareStack();
  __ la(T9, "HandleAtBytecode");
  __ jalr(T9);
  __ move(A2, S2);  // Delay-slot.
  RestoreStack();
  __ B(NEQ, V0, ZR, &done_);
  __ move(RA, S3);
  __ Jr(RA);
}

void InterpreterGeneratorMIPS::DoLoadLocal0() {
  LoadLocal(A0, 0);
  Push(A0);
  Dispatch(kLoadLocal0Length);
}

void InterpreterGeneratorMIPS::DoLoadLocal1() {
  LoadLocal(A0, 1);
  Push(A0);
  Dispatch(kLoadLocal1Length);
}

void InterpreterGeneratorMIPS::DoLoadLocal2() {
  LoadLocal(A0, 2);
  Push(A0);
  Dispatch(kLoadLocal2Length);
}

void InterpreterGeneratorMIPS::DoLoadLocal3() {
  LoadLocal(A0, 3);
  Push(A0);
  Dispatch(kLoadLocal3Length);
}

void InterpreterGeneratorMIPS::DoLoadLocal4() {
  LoadLocal(A0, 4);
  Push(A0);
  Dispatch(kLoadLocal4Length);
}

void InterpreterGeneratorMIPS::DoLoadLocal5() {
  LoadLocal(A0, 5);
  Push(A0);
  Dispatch(kLoadLocal5Length);
}

void InterpreterGeneratorMIPS::DoLoadLocal() {
  __ lbu(A0, Address(S1, 1));
  ShiftAddLoad(A0, S2, A0, TIMES_WORD_SIZE);
  Push(A0);
  Dispatch(kLoadLocalLength);
}

void InterpreterGeneratorMIPS::DoLoadLocalWide() {
  __ lw(A0, Address(S1, 1));
  ShiftAddLoad(A0, S2, A0, TIMES_WORD_SIZE);
  Push(A0);
  Dispatch(kLoadLocalWideLength);
}

void InterpreterGeneratorMIPS::DoLoadBoxed() {
  __ lbu(A0, Address(S1, 1));
  ShiftAddLoad(A1, S2, A0, TIMES_WORD_SIZE);
  __ lw(A0, Address(A1, Boxed::kValueOffset - HeapObject::kTag));
  Push(A0);
  Dispatch(kLoadBoxedLength);
}

void InterpreterGeneratorMIPS::DoLoadStatic() {
  __ lw(A0, Address(S1, 1));
  __ lw(A1, Address(S0, Process::kStaticsOffset));
  __ addiu(A1, A1, Immediate(Array::kSize - HeapObject::kTag));
  ShiftAddLoad(A0, A1, A0, TIMES_WORD_SIZE);
  Push(A0);
  Dispatch(kLoadStaticLength);
}

void InterpreterGeneratorMIPS::DoLoadStaticInit() {
  __ lw(A0, Address(S1, 1));
  __ lw(A1, Address(S0, Process::kStaticsOffset));
  __ addiu(A1, A1, Immediate(Array::kSize - HeapObject::kTag));
  ShiftAddLoad(A0, A1, A0, TIMES_WORD_SIZE);

  Label done;
  ASSERT(Smi::kTag == 0);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(EQ, T0, ZR, &done);

  __ lw(A1, Address(A0, HeapObject::kClassOffset - HeapObject::kTag));
  __ lw(A1, Address(A1, Class::kInstanceFormatOffset - HeapObject::kTag));

  int type = InstanceFormat::INITIALIZER_TYPE;
  __ andi(A1, A1, Immediate(InstanceFormat::TypeField::mask()));
  __ li(T0, Immediate(type << InstanceFormat::TypeField::shift()));
  __ B(NEQ, T0, A1, &done);

  // Invoke the initializer function.
  SaveByteCodePointer(A2);
  __ la(T9, "InterpreterMethodEntry");
  __ jalr(T9);
  __ lw(A0, Address(A0,
            Initializer::kFunctionOffset - HeapObject::kTag));  // Delay-slot.
  RestoreByteCodePointer(A2);

  __ Bind(&done);
  Push(A0);
  Dispatch(kLoadStaticInitLength);
}

void InterpreterGeneratorMIPS::DoLoadField() {
  __ lbu(A1, Address(S1, 1));
  LoadLocal(A0, 0);
  __ addiu(A0, A0, Immediate(Instance::kSize - HeapObject::kTag));
  ShiftAddLoad(A0, A0, A1, TIMES_WORD_SIZE);
  StoreLocal(A0, 0);
  Dispatch(kLoadFieldLength);
}

void InterpreterGeneratorMIPS::DoLoadFieldWide() {
  __ lw(A1, Address(S1, 1));
  LoadLocal(A0, 0);
  __ addiu(A0, A0, Immediate(Instance::kSize - HeapObject::kTag));
  ShiftAddLoad(A0, A0, A1, TIMES_WORD_SIZE);
  StoreLocal(A0, 0);
  Dispatch(kLoadFieldWideLength);
}

void InterpreterGeneratorMIPS::DoLoadConst() {
  __ lw(A0, Address(S1, 1));
  ShiftAddLoad(A2, S1, A0, TIMES_1);
  Push(A2);
  Dispatch(kLoadConstLength);
}

void InterpreterGeneratorMIPS::DoStoreLocal() {
  LoadLocal(A1, 0);
  __ lbu(A0, Address(S1, 1));
  ShiftAddStore(A1, S2, A0, TIMES_WORD_SIZE);
  Dispatch(kStoreLocalLength);
}

void InterpreterGeneratorMIPS::DoStoreBoxed() {
  LoadLocal(A2, 0);
  __ lbu(A0, Address(S1, 1));
  ShiftAddLoad(A1, S2, A0, TIMES_WORD_SIZE);
  __ sw(A2, Address(A1, Boxed::kValueOffset - HeapObject::kTag));

  AddToRememberedSet(A1, A2, A3);

  Dispatch(kStoreBoxedLength);
}

void InterpreterGeneratorMIPS::DoStoreStatic() {
  LoadLocal(A2, 0);
  __ lw(A0, Address(S1, 1));
  __ lw(A1, Address(S0, Process::kStaticsOffset));
  __ addiu(A3, A1, Immediate(Array::kSize - HeapObject::kTag));
  ShiftAddStore(A2, A3, A0, TIMES_WORD_SIZE);

  AddToRememberedSet(A1, A2, A0);

  Dispatch(kStoreStaticLength);
}

void InterpreterGeneratorMIPS::DoStoreField() {
  __ lbu(A1, Address(S1, 1));
  LoadLocal(A2, 0);
  LoadLocal(A0, 1);
  __ addiu(A3, A0, Immediate(Instance::kSize - HeapObject::kTag));
  ShiftAddStore(A2, A3, A1, TIMES_WORD_SIZE);
  DropNAndSetTop(1, A2);

  AddToRememberedSet(A0, A2, A3);

  Dispatch(kStoreFieldLength);
}

void InterpreterGeneratorMIPS::DoStoreFieldWide() {
  __ lw(A1, Address(S1, 1));
  LoadLocal(A2, 0);
  LoadLocal(A0, 1);
  __ addiu(A3, A0, Immediate(Instance::kSize - HeapObject::kTag));
  ShiftAddStore(A2, A3, A1, TIMES_WORD_SIZE);
  DropNAndSetTop(1, A2);

  AddToRememberedSet(A0, A2, A3);

  Dispatch(kStoreFieldWideLength);
}

void InterpreterGeneratorMIPS::DoLoadLiteralNull() {
  Push(S4);
  Dispatch(kLoadLiteralNullLength);
}

void InterpreterGeneratorMIPS::DoLoadLiteralTrue() {
  LoadTrue(A2);
  Push(A2);
  Dispatch(kLoadLiteralTrueLength);
}

void InterpreterGeneratorMIPS::DoLoadLiteralFalse() {
  LoadFalse(A2);
  Push(A2);
  Dispatch(kLoadLiteralFalseLength);
}

void InterpreterGeneratorMIPS::DoLoadLiteral0() {
  __ li(A0, Immediate(reinterpret_cast<int32_t>(Smi::FromWord(0))));
  Push(A0);
  Dispatch(kLoadLiteral0Length);
}

void InterpreterGeneratorMIPS::DoLoadLiteral1() {
  __ li(A0, Immediate(reinterpret_cast<int32_t>(Smi::FromWord(1))));
  Push(A0);
  Dispatch(kLoadLiteral1Length);
}

void InterpreterGeneratorMIPS::DoLoadLiteral() {
  __ lbu(A0, Address(S1, 1));
  __ sll(A0, A0, Immediate(Smi::kTagSize));
  ASSERT(Smi::kTag == 0);
  Push(A0);
  Dispatch(kLoadLiteralLength);
}

void InterpreterGeneratorMIPS::DoLoadLiteralWide() {
  ASSERT(Smi::kTag == 0);
  __ lw(A0, Address(S1, 1));
  __ sll(A0, A0, Immediate(Smi::kTagSize));
  Push(A0);
  Dispatch(kLoadLiteralWideLength);
}

void InterpreterGeneratorMIPS::DoInvokeMethodUnfold() {
  InvokeMethodUnfold(false);
}

void InterpreterGeneratorMIPS::DoInvokeMethod() {
  InvokeMethod(false);
}

void InterpreterGeneratorMIPS::DoInvokeNoSuchMethod() {
  // Use the noSuchMethod entry from entry zero of the virtual table.
  __ lw(A1, Address(S6, Program::kDispatchTableOffset));
  __ lw(A1, Address(A1, Array::kSize - HeapObject::kTag));

  SaveByteCodePointer(A2);
  __ la(T9, "InterpreterMethodEntry");
  __ jalr(T9);
  __ lw(A0, Address(A1,
                    DispatchTableEntry::kTargetOffset -
                        HeapObject::kTag));  // Delay-slot.
  RestoreByteCodePointer(A2);

  __ lw(S3, Address(S1, 1));
  // Compute the arity from the selector.
  ASSERT(Selector::ArityField::shift() == 0);
  __ andi(A2, S3, Immediate(Selector::ArityField::mask()));

  Drop(A2);

  StoreLocal(A0, 0);
  Dispatch(kInvokeNoSuchMethodLength);
}

void InterpreterGeneratorMIPS::DoInvokeTestNoSuchMethod() {
  LoadFalse(A2);
  StoreLocal(A2, 0);
  Dispatch(kInvokeTestNoSuchMethodLength);
}

void InterpreterGeneratorMIPS::DoInvokeTestUnfold() {
  InvokeMethodUnfold(true);
}

void InterpreterGeneratorMIPS::DoInvokeTest() {
  InvokeMethod(true);
}

void InterpreterGeneratorMIPS::DoInvokeStatic() {
  InvokeStatic();
}

void InterpreterGeneratorMIPS::DoInvokeFactory() {
  InvokeStatic();
}

void InterpreterGeneratorMIPS::DoInvokeLeafNative() {
  InvokeNative(false, false);
}

void InterpreterGeneratorMIPS::DoInvokeNative() {
  InvokeNative(false, true);
}

void InterpreterGeneratorMIPS::DoInvokeNativeYield() {
  InvokeNative(true, false);
}

void InterpreterGeneratorMIPS::DoInvokeSelector() {
  Label resume;
  SaveState(&resume);
  PrepareStack();
  __ la(T9, "HandleInvokeSelector");
  __ jalr(T9);
  __ move(A0, S0);  // Delay-slot.
  RestoreStack();
  __ move(A0, V0);
  RestoreState();
  __ Bind(&resume);

  SaveByteCodePointer(A2);
  __ la(T9, "InterpreterMethodEntry");
  __ Jalr(T9);
  RestoreByteCodePointer(A2);

  __ li(S3, Immediate(-2));
  __ lw(A2, Address(S1, 1));
  __ subu(S3, S3, A2);
  LoadFramePointer(A2);
  ShiftAddLoad(A2, A2, S3, TIMES_WORD_SIZE);

  __ sra(A2, A2, Immediate(1));
  ASSERT(Selector::ArityField::shift() == 0);
  __ andi(A2, A2, Immediate(Selector::ArityField::mask()));

  Drop(A2);

  StoreLocal(A0, 0);
  Dispatch(kInvokeSelectorLength);
}

void InterpreterGeneratorMIPS::InvokeEq(const char* fallback) {
  InvokeCompare(fallback, EQ);
}

void InterpreterGeneratorMIPS::InvokeLt(const char* fallback) {
  InvokeCompare(fallback, LT);
}

void InterpreterGeneratorMIPS::InvokeLe(const char* fallback) {
  InvokeCompare(fallback, LE);
}

void InterpreterGeneratorMIPS::InvokeGt(const char* fallback) {
  InvokeCompare(fallback, GT);
}

void InterpreterGeneratorMIPS::InvokeGe(const char* fallback) {
  InvokeCompare(fallback, GE);
}

void InterpreterGeneratorMIPS::InvokeAdd(const char* fallback) {
  Label no_overflow;
  LoadLocal(A0, 1);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 0);
  __ andi(T1, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T1, ZR, fallback);

  __ xor_(T1, A0, A1);
  __ b(LT, T1, ZR, &no_overflow);
  __ addu(T0, A0, A1);  // Delay-slot.
  __ xor_(T1, T0, A0);
  __ b(LT, T1, ZR, fallback);
  __ Bind(&no_overflow);
  __ move(A0, T0);  // Delay-slot.
  DropNAndSetTop(1, A0);
  Dispatch(kInvokeAddLength);
}

void InterpreterGeneratorMIPS::InvokeSub(const char* fallback) {
  Label no_overflow;
  LoadLocal(A0, 1);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 0);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);

  __ xor_(T1, A0, A1);
  __ srl(T1, T1, Immediate(31));
  __ b(EQ, T1, ZR, &no_overflow);
  __ subu(T0, A0, A1);  // Delay-slot.
  __ xor_(T1, T0, A0);
  __ b(LT, T1, ZR, fallback);
  __ Bind(&no_overflow);
  __ move(A0, T0);  // Delay-slot.
  DropNAndSetTop(1, A0);
  Dispatch(kInvokeSubLength);
}

void InterpreterGeneratorMIPS::InvokeMod(const char* fallback) {
  __ la(T9, fallback);
  __ Jr(T9);
}

void InterpreterGeneratorMIPS::InvokeMul(const char* fallback) {
  LoadLocal(A0, 1);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 0);
  __ andi(T1, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T1, ZR, fallback);

  __ sra(A0, A0, Immediate(1));
  __ mult(A0, A1);
  __ mfhi(T1);
  __ mflo(T0);
  __ move(A0, T0);
  __ sra(T0, T0, Immediate(31));
  __ B(NEQ, T1, T0, fallback);

  DropNAndSetTop(1, A0);
  Dispatch(kInvokeMulLength);
}

void InterpreterGeneratorMIPS::InvokeTruncDiv(const char* fallback) {
  __ la(T9, fallback);
  __ Jr(T9);
}

void InterpreterGeneratorMIPS::InvokeBitNot(const char* fallback) {
  LoadLocal(A0, 0);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);

  // Move negated.
  __ not_(A1, A0);
  // Bit clear the smi tag bit to smi tag again.
  __ xori(A1, A1, Immediate(Smi::kTagMask));

  StoreLocal(A1, 0);
  Dispatch(kInvokeBitNotLength);
}

void InterpreterGeneratorMIPS::InvokeBitAnd(const char* fallback) {
  LoadLocal(A0, 1);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 0);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);

  __ and_(A0, A0, A1);
  DropNAndSetTop(1, A0);
  Dispatch(kInvokeBitAndLength);
}

void InterpreterGeneratorMIPS::InvokeBitOr(const char* fallback) {
  LoadLocal(A0, 1);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 0);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  __ or_(A0, A0, A1);
  DropNAndSetTop(1, A0);
  Dispatch(kInvokeBitOrLength);
}

void InterpreterGeneratorMIPS::InvokeBitXor(const char* fallback) {
  LoadLocal(A0, 1);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 0);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  __ xor_(A0, A0, A1);
  DropNAndSetTop(1, A0);
  Dispatch(kInvokeBitXorLength);
}

void InterpreterGeneratorMIPS::InvokeBitShr(const char* fallback) {
  LoadLocal(A0, 1);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 0);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);

  // Untag and shift.
  __ sra(A0, A0, Immediate(1));
  __ sra(A1, A1, Immediate(1));

  Label shift;
  __ li(T0, Immediate(32));
  __ B(LT, A1, T0, &shift);
  __ li(A1, Immediate(31));
  __ Bind(&shift);
  __ srav(A0, A0, A1);

  // Retag and store.
  __ addu(A0, A0, A0);
  DropNAndSetTop(1, A0);
  Dispatch(kInvokeBitShrLength);
}

void InterpreterGeneratorMIPS::InvokeBitShl(const char* fallback) {
  LoadLocal(A0, 1);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 0);
  __ andi(T0, A1, Immediate(Smi::kTagSize));
  __ B(NEQ, T0, ZR, fallback);

  // Untag the shift count, but not the value. If the shift
  // count is greater than 31 (or negative), the shift is going
  // to misbehave so we have to guard against that.
  __ sra(A1, A1, Immediate(1));
  __ sra(T0, A1, Immediate(5));
  __ B(NEQ, T0, ZR, fallback);

  // Only allow to shift out "sign bits". If we shift
  // out any other bit, it's an overflow.
  __ sllv(A2, A0, A1);
  __ srav(A3, A2, A1);
  __ B(NEQ, A3, A0, fallback);

  DropNAndSetTop(1, A2);
  Dispatch(kInvokeBitShlLength);
}

void InterpreterGeneratorMIPS::DoPop() {
  Drop(1);
  Dispatch(kPopLength);
}

void InterpreterGeneratorMIPS::DoDrop() {
  __ lbu(A0, Address(S1, 1));
  Drop(A0);
  Dispatch(kDropLength);
}

void InterpreterGeneratorMIPS::DoReturn() {
  Return(false);
}

void InterpreterGeneratorMIPS::DoReturnNull() {
  Return(true);
}

void InterpreterGeneratorMIPS::DoBranchWide() {
  __ lw(A0, Address(S1, 1));
  __ addu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoBranchIfTrueWide() {
  Label branch;
  Pop(S3);
  LoadTrue(A2);
  __ B(EQ, S3, A2, &branch);
  Dispatch(kBranchIfTrueWideLength);

  __ Bind(&branch);
  __ lw(A0, Address(S1, 1));
  __ addu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoBranchIfFalseWide() {
  Label branch;
  Pop(S3);
  LoadTrue(A2);
  __ B(NEQ, S3, A2, &branch);
  Dispatch(kBranchIfFalseWideLength);

  __ Bind(&branch);
  __ lw(A0, Address(S1, 1));
  __ addu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoBranchBack() {
  CheckStackOverflow(0);
  __ lbu(A0, Address(S1, 1));
  __ subu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoBranchBackIfTrue() {
  CheckStackOverflow(0);

  Label branch;
  Pop(A1);
  LoadTrue(A2);
  __ B(EQ, A1, A2, &branch);
  Dispatch(kBranchBackIfTrueLength);
  __ Bind(&branch);
  __ lbu(A0, Address(S1, 1));
  __ subu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoBranchBackIfFalse() {
  CheckStackOverflow(0);

  Label branch;
  Pop(A1);
  LoadTrue(A2);
  __ B(NEQ, A1, A2, &branch);
  Dispatch(kBranchBackIfFalseLength);

  __ Bind(&branch);
  __ lbu(A0, Address(S1, 1));
  __ subu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoBranchBackWide() {
  CheckStackOverflow(0);
  __ lw(A0, Address(S1, 1));
  __ subu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoBranchBackIfTrueWide() {
  CheckStackOverflow(0);

  Label branch;
  Pop(A1);
  LoadTrue(A2);
  __ B(EQ, A2, A1, &branch);
  Dispatch(kBranchBackIfTrueWideLength);

  __ Bind(&branch);
  __ lw(A0, Address(S1, 1));
  __ subu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoBranchBackIfFalseWide() {
  CheckStackOverflow(0);

  Label branch;
  Pop(A1);
  LoadTrue(A2);
  __ B(NEQ, A2, A1, &branch);
  Dispatch(kBranchBackIfFalseWideLength);

  __ Bind(&branch);
  __ lw(A0, Address(S1, 1));
  __ subu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoPopAndBranchWide() {
  __ lbu(A0, Address(S1, 1));
  ShiftAdd(S2, S2, A0, TIMES_WORD_SIZE);

  __ lw(A0, Address(S1, 2));
  __ addu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoPopAndBranchBackWide() {
  CheckStackOverflow(0);

  __ lbu(A0, Address(S1, 1));
  ShiftAdd(S2, S2, A0, TIMES_WORD_SIZE);

  __ lw(A0, Address(S1, 2));
  __ subu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoAllocate() {
  Allocate(false);
}

void InterpreterGeneratorMIPS::DoAllocateImmutable() {
  Allocate(true);
}

void InterpreterGeneratorMIPS::DoAllocateBoxed() {
  LoadLocal(A1, 0);
  PrepareStack();
  __ la(T9, "HandleAllocateBoxed");
  __ jalr(T9);
  __ move(A0, S0);  // Delay-slot.
  RestoreStack();
  __ move(A0, V0);
  __ andi(A1, A0, Immediate(Failure::kTagMask | Failure::kTypeMask));
  __ li(T0, Immediate(Failure::kTag));
  __ B(EQ, A1, T0, &gc_);
  StoreLocal(A0, 0);
  Dispatch(kAllocateBoxedLength);
}

void InterpreterGeneratorMIPS::DoNegate() {
  LoadLocal(A1, 0);
  LoadTrue(A2);
  AddIf(A1, A2, EQ, T0, T1, A2, kFalseOffset - kTrueOffset);
  StoreLocal(A2, 0);
  Dispatch(kNegateLength);
}

void InterpreterGeneratorMIPS::DoStackOverflowCheck() {
  __ lw(A0, Address(S1, 1));
  __ lw(A1, Address(S0, Process::kStackLimitOffset));
  ShiftSub(A3, S2, A0, TIMES_WORD_SIZE);
  __ sltu(T0, A1, A3);
  __ B(EQ, T0, ZR, &check_stack_overflow_);
  Dispatch(kStackOverflowCheckLength);
}

void InterpreterGeneratorMIPS::DoThrowAfterSaveState(Label* resume) {
  // Use the stack to store the stack delta initialized to zero, and the
  // frame pointer return value.
  __ addiu(SP, SP, Immediate(-2 * kWordSize));
  __ addiu(A2, SP, Immediate(kWordSize));
  __ sw(ZR, Address(A2, 0));
  __ move(T0, SP);

  __ move(A0, S0);
  __ move(A1, S3);
  PrepareStack();
  __ la(T9, "HandleThrow");
  __ jalr(T9);
  __ move(A3, T0);  // Delay-slot.
  RestoreStack();
  // Load results and restore SP, before restoring state.
  __ lw(A2, Address(SP, 0));
  __ lw(A3, Address(SP, kWordSize));
  __ addiu(SP, SP, Immediate(2 * kWordSize));

  RestoreState();
  __ Bind(resume);

  Label unwind;
  __ B(NEQ, V0, ZR, &unwind);

  __ b(&done_);
  __ ori(V0, ZR, Immediate(Interpreter::kUncaughtException));  // Delay-slot.
  __ Bind(&unwind);
  StoreFramePointer(A2);
  __ move(S1, V0);
  ShiftAdd(S2, S2, A3, TIMES_WORD_SIZE);

  StoreLocal(S3, 0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoThrow() {
  // Load object into callee-save register not touched by
  // save and restore state.
  LoadLocal(S3, 0);
  Label resume;
  SaveState(&resume);
  DoThrowAfterSaveState(&resume);
}

void InterpreterGeneratorMIPS::DoSubroutineCall() {
  __ lw(A0, Address(S1, 1));
  __ lw(A1, Address(S1, 5));

  // Push the return delta as a tagged smi.
  ASSERT(Smi::kTag == 0);
  __ sll(A1, A1, Immediate(Smi::kTagSize));
  Push(A1);

  __ addu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoSubroutineReturn() {
  Pop(A0);
  __ srl(A0, A0, Immediate(Smi::kTagSize));
  __ subu(S1, S1, A0);
  Dispatch(0);
}

void InterpreterGeneratorMIPS::DoProcessYield() {
  LoadLocal(A0, 0);
  __ sra(V0, A0, Immediate(1));
  __ addiu(S1, S1, Immediate(kProcessYieldLength));
  StoreLocal(S4, 0);
  __ B(&done_);
}

void InterpreterGeneratorMIPS::DoCoroutineChange() {
  LoadLocal(S3, 0);
  LoadLocal(A1, 1);

  StoreLocal(S4, 0);
  StoreLocal(S4, 1);

  Label resume;
  SaveState(&resume);
  PrepareStack();
  __ la(T9, "HandleCoroutineChange");
  __ jalr(T9);
  __ move(A0, S0);  // Delay-slot.
  RestoreStack();
  RestoreState();
  __ Bind(&resume);
  __ Bind("", "InterpreterCoroutineEntry");

  DropNAndSetTop(1, S3);
  Dispatch(kCoroutineChangeLength);
}

void InterpreterGeneratorMIPS::DoIdentical() {
  LoadLocal(A0, 0);
  LoadLocal(A1, 1);

  Label fast_case;
  Label bail_out;

  // If either is a smi they are not both doubles or large integers.
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(EQ, T0, ZR, &fast_case);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(EQ, T0, ZR, &fast_case);

  // If they do not have the same type they are not both double or
  // large integers.
  __ lw(A2, Address(A0, HeapObject::kClassOffset - HeapObject::kTag));
  __ lw(A2, Address(A2, Class::kInstanceFormatOffset - HeapObject::kTag));
  __ lw(A3, Address(A1, HeapObject::kClassOffset - HeapObject::kTag));
  __ lw(A3, Address(A3, Class::kInstanceFormatOffset - HeapObject::kTag));
  __ B(NEQ, A2, A3, &fast_case);

  int double_type = InstanceFormat::DOUBLE_TYPE;
  int large_integer_type = InstanceFormat::LARGE_INTEGER_TYPE;
  int type_field_shift = InstanceFormat::TypeField::shift();

  __ andi(A2, A2, Immediate(InstanceFormat::TypeField::mask()));
  __ li(T0, Immediate(double_type << type_field_shift));
  __ B(EQ, T0, A2, &bail_out);
  __ li(T0, Immediate(large_integer_type << type_field_shift));
  __ B(EQ, T0, A2, &bail_out);

  __ Bind(&fast_case);
  ToBoolean(A1, A0, EQ, A2);
  StoreLocal(A2, 1);
  Drop(1);
  Dispatch(kIdenticalLength);

  __ Bind(&bail_out);
  __ move(A2, A0);
  PrepareStack();
  __ la(T9, "HandleIdentical");
  __ jalr(T9);
  __ move(A0, S0);  // Delay-slot.
  RestoreStack();
  DropNAndSetTop(1, V0);
  Dispatch(kIdenticalLength);
}

void InterpreterGeneratorMIPS::DoIdenticalNonNumeric() {
  LoadLocal(A0, 0);
  LoadLocal(A1, 1);
  ToBoolean(A0, A1, EQ, A2);
  StoreLocal(A2, 1);

  Drop(1);
  Dispatch(kIdenticalNonNumericLength);
}

void InterpreterGeneratorMIPS::DoEnterNoSuchMethod() {
  SaveState(&interpreter_entry_);
  PrepareStack();
  __ la(T9, "HandleEnterNoSuchMethod");
  __ jalr(T9);
  __ move(A0, S0);  // Delay-slot.
  RestoreStack();
  RestoreState();
}

void InterpreterGeneratorMIPS::DoExitNoSuchMethod() {
  Pop(A0);  // Result.
  Pop(A1);  // Selector.
  __ srl(A1, A1, Immediate(Smi::kTagSize));

  LoadFramePointer(S2);

  Pop(A2);
  StoreFramePointer(A2);

  Label done;
  __ andi(A2, A1, Immediate(Selector::KindField::mask()));
  __ li(T0,  Immediate(Selector::SETTER << Selector::KindField::shift()));
  __ B(NEQ, T0, A2, &done);

  // Setter argument is at offset 1, as we still have the return address on the
  // stack.
  LoadLocal(A0, 1);

  __ Bind(&done);
  Pop(RA);
  __ move(T9, RA);
  __ Jr(T9);
}

void InterpreterGeneratorMIPS::DoMethodEnd() {
  __ break_();
}

void InterpreterGeneratorMIPS::DoIntrinsicObjectEquals() {
  LoadLocal(A0, 0);
  LoadLocal(A1, 1);
  ToBoolean(A0, A1, EQ, A2);
  StoreLocal(A2, -1);
  Drop(1);
  Dispatch(kInvokeMethodLength);
}

void InterpreterGeneratorMIPS::DoIntrinsicGetField() {
  __ lbu(A1, Address(A0, 2 + Function::kSize - HeapObject::kTag));
  LoadLocal(A0, 0);
  __ addiu(A0, A0, Immediate(Instance::kSize - HeapObject::kTag));
  ShiftAddLoad(A0, A0, A1, TIMES_WORD_SIZE);

  __ Jr(RA);
}

void InterpreterGeneratorMIPS::DoIntrinsicSetField() {
  __ lbu(A1, Address(A0, 3 + Function::kSize - HeapObject::kTag));
  LoadLocal(S3, 0);
  LoadLocal(A2, 1);
  __ addiu(A3, A2, Immediate(Instance::kSize - HeapObject::kTag));
  ShiftAddStore(S3, A3, A1, TIMES_WORD_SIZE);

  // S3 is not trashed.
  AddToRememberedSet(A2, S3, A3);

  __ move(T9, RA);
  __ jr(T9);
  __ move(A0, S3);  // Delay-slot.
}

void InterpreterGeneratorMIPS::DoIntrinsicListIndexGet() {
  LoadLocal(A1, 0);  // Index.
  LoadLocal(A2, 1);  // List.

  Label failure;
  ASSERT(Smi::kTag == 0);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, &intrinsic_failure_);
  __ B(LT, A1, ZR, &intrinsic_failure_);

  // Load the backing store (array) from the first instance field of the list.
  __ lw(A2, Address(A2, Instance::kSize - HeapObject::kTag));
  __ lw(A3, Address(A2, Array::kLengthOffset - HeapObject::kTag));

  // Check the index against the length.
  __ B(GE, A1, A3, &intrinsic_failure_);

  // Load from the array and continue.
  ASSERT(Smi::kTagSize == 1);
  __ addiu(A2, A2, Immediate(Array::kSize - HeapObject::kTag));
  ShiftAddLoad(A0, A2, A1, TIMES_2);

  __ Jr(RA);
}

void InterpreterGeneratorMIPS::DoIntrinsicListIndexSet() {
  LoadLocal(A1, 1);  // Index.
  LoadLocal(A2, 2);  // List.

  ASSERT(Smi::kTag == 0);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, &intrinsic_failure_);
  __ B(LT, A1, ZR, &intrinsic_failure_);

  // Load the backing store (array) from the first instance field of the list.
  __ lw(A2, Address(A2, Instance::kSize - HeapObject::kTag));
  __ lw(A3, Address(A2, Array::kLengthOffset - HeapObject::kTag));

  // Check the index against the length.
  __ B(GE, A1, A3, &intrinsic_failure_);

  // Store to the array and continue.
  ASSERT(Smi::kTagSize == 1);
  LoadLocal(S3, 0);
  __ addiu(T4, A2, Immediate(Array::kSize - HeapObject::kTag));
  ShiftAddStore(S3, T4, A1, TIMES_2);
  __ move(S5, RA);

  // S3 is not trashed.
  AddToRememberedSet(A2, S3, A3);

  __ move(T9, S5);
  __ jr(T9);
  __ move(A0, S3);  // Delay-slot.
}

void InterpreterGeneratorMIPS::DoIntrinsicListLength() {
  // Load the backing store (array) from the first instance field of the list.
  LoadLocal(A2, 0);  // List.
  __ lw(A2, Address(A2, Instance::kSize - HeapObject::kTag));
  __ jr(RA);
  __ lw(A0, Address(A2, Array::kLengthOffset - HeapObject::kTag));  // D-slot.
}

void InterpreterGeneratorMIPS::Pop(Register reg) {
  __ lw(reg, Address(S2, 0));
  __ addiu(S2, S2, Immediate(1 * kWordSize));
}

void InterpreterGeneratorMIPS::Push(Register reg) {
  __ addiu(S2, S2, Immediate(-1 * kWordSize));
  __ sw(reg, Address(S2, 0));
}

void InterpreterGeneratorMIPS::Return(bool is_return_null) {
  // Materialize the result in register A0.
  if (is_return_null) {
    __ move(A0, S4);
  } else {
    LoadLocal(A0, 0);
  }

  LoadFramePointer(S2);

  Pop(A2);
  StoreFramePointer(A2);

  Pop(RA);
  __ Jr(RA);
}

void InterpreterGeneratorMIPS::LoadLocal(Register reg, int index) {
  __ lw(reg, Address(S2, index * kWordSize));
}

void InterpreterGeneratorMIPS::StoreLocal(Register reg, int index) {
  __ sw(reg, Address(S2, index * kWordSize));
}

void InterpreterGeneratorMIPS::Drop(int n) {
  __ addiu(S2, S2, Immediate(n * kWordSize));
}

void InterpreterGeneratorMIPS::Drop(Register reg) {
  ShiftAdd(S2, S2, reg, TIMES_WORD_SIZE);
}

void InterpreterGeneratorMIPS::DropNAndSetTop(int dropping_slots,
                                              Register reg) {
  __ addiu(S2, S2, Immediate(dropping_slots * kWordSize));
  __ sw(reg, Address(S2, 0));
}

void InterpreterGeneratorMIPS::LoadFramePointer(Register reg) {
  __ lw(reg, Address(SP, spill_size_));
}

void InterpreterGeneratorMIPS::StoreFramePointer(Register reg) {
  __ sw(reg, Address(SP, spill_size_));
}

void InterpreterGeneratorMIPS::SaveByteCodePointer(Register scratch) {
  LoadFramePointer(scratch);
  __ sw(S1, Address(scratch, -kWordSize));
}

void InterpreterGeneratorMIPS::RestoreByteCodePointer(Register scratch) {
  LoadFramePointer(scratch);
  __ lw(S1, Address(scratch, -kWordSize));
}

void InterpreterGeneratorMIPS::PushFrameDescriptor(Register return_address,
                                                   Register scratch) {
  LoadFramePointer(scratch);
  __ sw(return_address, Address(scratch, -kWordSize));

  __ li(scratch, Immediate(0));
  Push(scratch);

  LoadFramePointer(scratch);
  Push(scratch);

  StoreFramePointer(S2);

  __ li(scratch, Immediate(0));
  Push(scratch);
}

void InterpreterGeneratorMIPS::ReadFrameDescriptor(Register scratch) {
  LoadFramePointer(S2);
  // Store old frame pointer from stack.
  LoadLocal(scratch, 0);
  StoreFramePointer(scratch);

  // Load return address.
  __ lw(S1, Address(scratch, -kWordSize));
}

void InterpreterGeneratorMIPS::InvokeMethodUnfold(bool test) {
  // Get the selector from the bytecodes.
  __ lw(S3, Address(S1, 1));

  if (test) {
  // Get the receiver from the stack.
    LoadLocal(A1, 0);
  } else {
    // Compute the arity from the selector.
    ASSERT(Selector::ArityField::shift() == 0);
    __ andi(A2, S3, Immediate(Selector::ArityField::mask()));

    // Get the receiver from the stack.
    ShiftAddLoad(A1, S2, A2, TIMES_WORD_SIZE);
  }

  // Compute the receiver class.
  Label smi, probe;
  ASSERT(Smi::kTag == 0);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(EQ, T0, ZR, &smi);
  __ lw(A2, Address(A1, HeapObject::kClassOffset - HeapObject::kTag));

  // Find the entry in the primary lookup cache.
  Label miss, finish;
  ASSERT(Utils::IsPowerOfTwo(LookupCache::kPrimarySize));
  ASSERT(sizeof(LookupCache::Entry) == 1 << 4);
  __ Bind(&probe);
  __ xor_(A3, A2, S3);
  __ li(A0, Immediate(LookupCache::kPrimarySize - 1));
  __ and_(A0, A3, A0);
  __ lw(A3, Address(S0, Process::kPrimaryLookupCacheOffset));
  ShiftAdd(A0, A3, A0, 4);

  // Validate the primary entry.
  __ lw(A3, Address(A0, LookupCache::kClassOffset));
  __ B(NEQ, A2, A3,  &miss);
  __ lw(A3, Address(A0, LookupCache::kSelectorOffset));
  __ B(NEQ, S3, A3, &miss);

  // At this point, we've got our hands on a valid lookup cache entry.
  __ Bind(&finish);
  if (test) {
    __ lw(A0, Address(A0, LookupCache::kCodeOffset));
  } else {
    Label hit;
    // TODO(petarj): Handle intrinsics.
    __ lw(A1, Address(A0, LookupCache::kCodeOffset));
    __ lw(A0, Address(A0, LookupCache::kTargetOffset));

    __ and_(T0, A0, A0);
    __ B(NEQ, T0, ZR, &hit);
    __ la(A0, "InterpreterMethodEntry");
    __ Bind(&hit);
  }

  if (test) {
    // Materialize either true or false depending on whether or not
    // we've found a target method.
    Label found;
    ToBoolean(A0, ZR, EQ, A2);
    StoreLocal(A2, 0);
    Dispatch(kInvokeTestUnfoldLength);
  } else {
    SaveByteCodePointer(A2);
    __ la(T9, "InterpreterMethodEntry");
    __ Jalr(T9);
    RestoreByteCodePointer(A2);

    __ lw(S3, Address(S1, 1));
    // Compute the arity from the selector.
    ASSERT(Selector::ArityField::shift() == 0);
    __ andi(A2, S3, Immediate(Selector::ArityField::mask()));

    Drop(A2);

    StoreLocal(A0, 0);
    Dispatch(kInvokeMethodUnfoldLength);
  }

  __ Bind(&smi);
  __ lw(A2, Address(S6, Program::kSmiClassOffset));
  __ B(&probe);

  // We didn't find a valid entry in primary lookup cache.
  __ Bind(&miss);
  // Arguments:
  // - A0: process
  // - A1: primary cache entry
  // - A2: class (already in A2)
  // - A3: selector
  __ move(A1, A0);
  __ move(A0, S0);
  PrepareStack();
  __ la(T9, "HandleLookupEntry");
  __ jalr(T9);
  __ move(A3, S3);  // Delay-slot.
  RestoreStack();

  __ b(&finish);
  __ move(A0, V0);  // Delay-slot.
}

void InterpreterGeneratorMIPS::InvokeMethod(bool test) {
  // Get the selector from the bytecodes.
  __ lw(S3, Address(S1, 1));

  __ lw(A1, Address(S6, Program::kDispatchTableOffset));

  if (!test) {
    // Compute the arity from the selector.
    ASSERT(Selector::ArityField::shift() == 0);
    __ andi(A2, S3, Immediate(Selector::ArityField::mask()));
  }

  // Compute the selector offset (smi tagged) from the selector.
  __ li(S5, Immediate(Selector::IdField::mask()));
  __ and_(S3, S3, S5);
  __ srl(S3, S3, Immediate(Selector::IdField::shift() - Smi::kTagSize));

  // Get the receiver from the stack.
  if (test) {
    LoadLocal(A2, 0);
  } else {
    ShiftAddLoad(A2, S2, A2, TIMES_WORD_SIZE);
  }

  // Compute the receiver class.
  Label smi, dispatch;
  ASSERT(Smi::kTag == 0);
  __ andi(T0, A2, Immediate(Smi::kTagMask));
  __ B(EQ, T0, ZR, &smi);
  __ lw(A2, Address(A2, HeapObject::kClassOffset - HeapObject::kTag));

  // Compute entry index: class id + selector offset.
  int id_offset = Class::kIdOrTransformationTargetOffset - HeapObject::kTag;
  __ Bind(&dispatch);
  __ lw(A2, Address(A2, id_offset));
  __ addu(A2, A2, S3);

  // Fetch the entry from the table. Because the index is smi tagged
  // we only multiply by two -- not four -- when indexing.
  ASSERT(Smi::kTagSize == 1);
  __ addiu(A1, A1, Immediate(Array::kSize - HeapObject::kTag));
  ShiftAddLoad(A1, A1, A2, TIMES_2);

  // Validate that the offset stored in the entry matches the offset
  // we used to find it.
  Label invalid;
  __ lw(A3, Address(A1, DispatchTableEntry::kOffsetOffset - HeapObject::kTag));
  __ B(NEQ, S3, A3, &invalid);

  // Load the target and the intrinsic from the entry.
  Label validated, intrinsified;
  if (test) {
    // Valid entry: The answer is true.
    LoadTrue(A2);
    StoreLocal(A2, 0);
    Dispatch(kInvokeTestLength);
  } else {
    __ Bind(&validated);

    __ lw(A0, Address(A1,
                      DispatchTableEntry::kTargetOffset - HeapObject::kTag));

    SaveByteCodePointer(A2);
    __ lw(A1, Address(A1, DispatchTableEntry::kCodeOffset - HeapObject::kTag));
    __ move(T9, A1);
    __ Jalr(T9);
    RestoreByteCodePointer(A2);

    __ lw(S3, Address(S1, 1));
    // Compute the arity from the selector.
    ASSERT(Selector::ArityField::shift() == 0);
    __ andi(A2, S3, Immediate(Selector::ArityField::mask()));

    Drop(A2);

    StoreLocal(A0, 0);
    Dispatch(kInvokeMethodLength);
  }

  __ Bind(&smi);
  __ b(&dispatch);
  __ lw(A2, Address(S6, Program::kSmiClassOffset));  // Delay-slot.

  if (test) {
    // Invalid entry: The answer is false.
    __ Bind(&invalid);
    LoadFalse(A2);
    StoreLocal(A2, 0);
    Dispatch(kInvokeTestLength);
  } else {
    __ Bind(&intrinsified);
    __ move(T9, A2);
    __ Jr(T9);

    // Invalid entry: Use the noSuchMethod entry from entry zero of
    // the virtual table.
    __ Bind(&invalid);
    __ lw(A1, Address(S6, Program::kDispatchTableOffset));
    __ b(&validated);
    __ lw(A1, Address(A1, Array::kSize - HeapObject::kTag));  // Delay-slot.
  }
}

void InterpreterGeneratorMIPS::InvokeNative(bool yield, bool safepoint) {
  __ lbu(A1, Address(S1, 1));
  // Also skip two empty slots.
  __ addiu(A1, A1, Immediate(2));
  __ lbu(A0, Address(S1, 2));

  // Load native from native table.
  __ la(S5, "kNativeTable");
  ShiftAddLoad(A2, S5, A0, TIMES_WORD_SIZE);

  // Setup argument (process and pointer to first argument).
  ShiftAdd(S3, S2, A1, TIMES_WORD_SIZE);
  __ move(A1, S3);
  __ move(A0, S0);
  Label continue_with_result;
  if (safepoint) SaveState(&continue_with_result);
  PrepareStack();
  __ move(T9, A2);
  __ Jalr(T9);
  RestoreStack();
  // Result is now in v0.
  __ move(A0, V0);

  if (safepoint) RestoreState();
  __ Bind(&continue_with_result);
  Label failure;
  __ andi(A1, A0, Immediate(Failure::kTagMask));
  __ li(T0, Immediate(Failure::kTag));
  __ B(EQ, A1, T0, &failure);

  if (yield) {
    ASSERT(!safepoint);
    // If the result of calling the native is null, we don't yield.
    Label dont_yield;
    __ B(EQ, A0, S4, &dont_yield);

    // Yield to the target port.
    __ lw(A3, Address(SP, 0));
    __ sw(A0, Address(A3, 0));
    __ li(V0, Immediate(Interpreter::kTargetYield));

    SaveState(&dont_yield);
    __ B(&done_state_saved_);

    __ Bind(&dont_yield);
  }

  LoadFramePointer(S2);

  Pop(A2);
  StoreFramePointer(A2);

  Pop(RA);
  __ move(T9, RA);
  __ Jr(T9);

  // Failure: Check if it's a request to garbage collect. If not,
  // just continue running the failure block by dispatching to the
  // next bytecode.
  __ Bind(&failure);
  __ andi(A1, A0, Immediate(Failure::kTagMask | Failure::kTypeMask));
  __ li(T0, Immediate(Failure::kTag));
  __ B(EQ, A1, T0, &gc_);

  __ move(A1, A0);
  PrepareStack();
  __ la(T9, "HandleObjectFromFailure");
  __ jalr(T9);
  __ move(A0, S0);  // Delay-slot.
  RestoreStack();
  Push(V0);
  Dispatch(kInvokeNativeLength);
}

void InterpreterGeneratorMIPS::InvokeStatic() {
  __ lw(A1, Address(S1, 1));
  ShiftAddLoad(A0, S1, A1, TIMES_1);

  // Compute and push the return address on the stack.
  SaveByteCodePointer(A2);
  __ la(T9, "InterpreterMethodEntry");
  __ Jalr(T9);
  RestoreByteCodePointer(A2);

  __ lw(A1, Address(S1, 1));
  ShiftAddLoad(A1, S1, A1, TIMES_1);

  // Read the arity from the function. Note that the arity is smi tagged.
  __ lw(A1, Address(A1, Function::kArityOffset - HeapObject::kTag));
  __ sra(A1, A1, Immediate(Smi::kTagSize));

  Drop(A1);

  Push(A0);
  Dispatch(kInvokeStaticLength);
}

void InterpreterGeneratorMIPS::Allocate(bool immutable) {
  __ lw(A0, Address(S1, 1));
  ShiftAddLoad(S3, S1, A0, TIMES_1);

  const Register kRegisterAllocateImmutable = S5;

  // Initialization of [kRegisterAllocateImmutable] depends on [immutable]
  __ li(kRegisterAllocateImmutable, Immediate(immutable ? 1 : 0));

  // Loop over all arguments and find out if all of them are immutable (then we
  // can set the immutable bit in this object too).
  Label allocate;
  if (immutable) {
    __ lw(A2, Address(S3, Class::kInstanceFormatOffset - HeapObject::kTag));
    __ li(A3, Immediate(InstanceFormat::FixedSizeField::mask()));
    __ and_(A2, A2, A3);
    int size_shift = InstanceFormat::FixedSizeField::shift() - kPointerSizeLog2;
    __ sra(A2, A2, Immediate(size_shift));

    // A2 = SizeOfEntireObject - Instance::kSize
    __ addiu(A2, A2, Immediate(-1 * Instance::kSize));

    // A3 = StackPointer(S2) + NumberOfFields * kPointerSize
    __ addu(A3, S2, A2);

    Label loop;
    Label break_loop_with_mutable_field;

    // Decrement pointer to point to next field.
    __ Bind(&loop);
    __ addiu(A3, A3, Immediate(-1 * kPointerSize));

    // Test whether S2 > A3. If so we're done and it's immutable.
    __ sltu(T0, A3, S2);
    __ B(GT, T0, ZR, &allocate);

    // If Smi, continue the loop.
    __ lw(A2, Address(A3, 0));
    __ andi(T0, A2, Immediate(Smi::kTagMask));
    __ B(EQ, T0, ZR, &loop);

    // Load class of object we want to test immutability of.
    __ lw(A0, Address(A2, HeapObject::kClassOffset - HeapObject::kTag));

    // Load instance format & handle the three cases:
    //  - never immutable (based on instance format) => not immutable
    //  - always immutable (based on instance format) => immutable
    //  - else (only instances) => check runtime-tracked bit
    uword mask = InstanceFormat::ImmutableField::mask();
    uword always_immutable_mask = InstanceFormat::ImmutableField::encode(
        InstanceFormat::ALWAYS_IMMUTABLE);
    uword never_immutable_mask =
        InstanceFormat::ImmutableField::encode(InstanceFormat::NEVER_IMMUTABLE);

    __ lw(A0, Address(A0, Class::kInstanceFormatOffset - HeapObject::kTag));
    __ li(A1, Immediate(mask));
    __ and_(A0, A0, A1);

    // If this type is never immutable we break the loop.
    __ li(T0, Immediate(never_immutable_mask));
    __ B(EQ, A0, T0, &break_loop_with_mutable_field);

    // If this is type is always immutable we continue the loop.
    __ li(T0, Immediate(always_immutable_mask));
    __ B(EQ, A0, T0, &loop);

    // Else, we must have an Instance and check the runtime-tracked
    // immutable bit.
    uword im_mask = Instance::FlagsImmutabilityField::encode(true);
    __ lw(A2, Address(A2, Instance::kFlagsOffset - HeapObject::kTag));
    __ andi(A2, A2, Immediate(im_mask));
    __ li(T0, Immediate(im_mask));
    __ B(EQ, A2, T0, &loop);

    __ Bind(&break_loop_with_mutable_field);

    __ li(kRegisterAllocateImmutable, Immediate(0));
    // Fall through
  }

  __ Bind(&allocate);
  __ move(A0, S0);
  __ move(A1, S3);
  PrepareStack();
  __ la(T9, "HandleAllocate");
  __ jalr(T9);
  __ move(A2, kRegisterAllocateImmutable);  // Delay-slot.
  RestoreStack();
  __ move(A0, V0);
  __ andi(A1, A0, Immediate(Failure::kTagMask | Failure::kTypeMask));
  __ li(T0, Immediate(Failure::kTag));
  __ B(EQ, A1, T0, &gc_);
  __ lw(A2, Address(S3, Class::kInstanceFormatOffset - HeapObject::kTag));
  __ li(A3, Immediate(InstanceFormat::FixedSizeField::mask()));
  __ and_(A2, A2, A3);
  // The fixed size is recorded as the number of pointers. Therefore, the
  // size in bytes is the recorded size multiplied by kPointerSize. Instead
  // of doing the multiplication we shift right by kPointerSizeLog2 less.
  ASSERT(InstanceFormat::FixedSizeField::shift() >= kPointerSizeLog2);
  int size_shift = InstanceFormat::FixedSizeField::shift() - kPointerSizeLog2;
  __ srl(A2, A2, Immediate(size_shift));

  // Compute the address of the first and last instance field.
  __ addiu(S3, A0, Immediate(-1 * (kWordSize + HeapObject::kTag)));
  __ addu(S3, S3, A2);
  __ addiu(S5, A0, Immediate(Instance::kSize - HeapObject::kTag));

  Label loop, done;
  __ Bind(&loop);
  __ sltu(T0, S3, S5);
  __ B(GT, T0, ZR, &done);
  Pop(A1);
  __ sw(A1, Address(S3, 0));
  __ b(&loop);
  __ addiu(S3, S3, Immediate(-1 * kWordSize));  // Delay-slot.

  __ Bind(&done);
  Push(A0);
  Dispatch(kAllocateLength);
}

void InterpreterGeneratorMIPS::AddToRememberedSet(Register object,
                                                  Register value,
                                                  Register scratch) {
  Label smi;
  __ andi(T0, value, Immediate(Smi::kTagMask));
  __ B(EQ, T0, ZR, &smi);

  __ lw(scratch, Address(S0, Process::kRememberedSetBiasOffset));
  ShiftRightAdd(scratch, scratch, object, GCMetadata::kCardSizeLog2);
  // This will never store zero (kNoNewSpacePointers) because the object is
  // tagged!
  __ sb(object, Address(scratch, 0));

  __ Bind(&smi);
}

void InterpreterGeneratorMIPS::InvokeCompare(const char* fallback,
                                             Condition cond) {
  LoadLocal(A0, 0);
  __ andi(T0, A0, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);
  LoadLocal(A1, 1);
  __ andi(T0, A1, Immediate(Smi::kTagMask));
  __ B(NEQ, T0, ZR, fallback);

  ToBoolean(A1, A0, cond, A2);
  DropNAndSetTop(1, A2);
  Dispatch(5);

  DropNAndSetTop(1, S6);
  Dispatch(5);
}

void InterpreterGeneratorMIPS::AddIf(Register reg1, Register reg2,
                                     Condition cond, Register scratch1,
                                     Register scratch2, Register dest,
                                     int value) {
  if (cond == GE) {
    __ addiu(scratch2, dest, Immediate(value));
    __ slt(scratch1, reg1, reg2);
    __ movz(dest, scratch2, scratch1);
  } else if (cond == LE) {
    __ addiu(scratch2, dest, Immediate(value));
    __ slt(scratch1, reg2, reg1);
    __ movz(dest, scratch2, scratch1);
  } else if (cond == EQ) {
    __ xor_(scratch1, reg1, reg2);
    __ addiu(scratch2, dest, Immediate(value));
    __ movz(dest, scratch2, scratch1);
  } else if (cond == LT) {
    __ slt(scratch1, reg1, reg2);
    if (Utils::IsPowerOfTwo(value)) {
      __ sll(scratch1, scratch1, Immediate(Utils::HighestBit(value)));
      if (value > 0) {
        __ addu(dest, dest, scratch1);
      } else {
        __ subu(dest, dest, scratch1);
      }
    } else {
      __ addiu(scratch2, dest, Immediate(value));
      __ movn(dest, scratch2, scratch1);
    }
  } else if (cond == GT) {
    __ slt(scratch1, reg2, reg1);
    if (Utils::IsPowerOfTwo(value)) {
      __ sll(scratch1, scratch1, Immediate(Utils::HighestBit(value)));
      if (value > 0) {
        __ addu(dest, dest, scratch1);
      } else {
        __ subu(dest, dest, scratch1);
      }
    } else {
      __ addiu(scratch2, dest, Immediate(value));
      __ movn(dest, scratch2, scratch1);
    }
  } else {
    Label done, add;
    __ B(cond, reg1, reg2, &add);
    __ B(&done);
    __ Bind(&add);
    __ addiu(dest, dest, Immediate(value));
    __ Bind(&done);
  }
}

void InterpreterGeneratorMIPS::LoadTrue(Register reg) {
  __ addiu(reg, S4, Immediate(kTrueOffset));
}

void InterpreterGeneratorMIPS::LoadFalse(Register reg) {
  __ addiu(reg, S4, Immediate(kFalseOffset));
}

void InterpreterGeneratorMIPS::ToBoolean(Register reg1,
                                         Register reg2,
                                         Condition cond,
                                         Register reg) {
  LoadFalse(reg);
  AddIf(reg1, reg2, cond, T0, T1, reg, kTrueOffset - kFalseOffset);
}

void InterpreterGeneratorMIPS::CheckStackOverflow(int size) {
  __ lw(A1, Address(S0, Process::kStackLimitOffset));
  __ sltu(T0, A1, S2);
  if (size == 0) {
    __ B(EQ, T0, ZR, &check_stack_overflow_0_);
  } else {
    Label done;
    __ B(GT, T0, ZR, &done);
    __ b(&check_stack_overflow_);
    __ ori(A0, ZR, Immediate(size));  // Delay-slot.
    __ Bind(&done);
  }
}

void InterpreterGeneratorMIPS::Dispatch(int size) {
  __ lbu(S3, Address(S1, size));
  if (size > 0) {
    __ addiu(S1, S1, Immediate(size));
  }
  ShiftAddJump(S7, S3, TIMES_WORD_SIZE);
}

void InterpreterGeneratorMIPS::SaveState(Label* resume) {
  // Save the bytecode pointer at the return-address slot.
  LoadFramePointer(A3);
  __ sw(S1, Address(A3, -kWordSize));

  // Push resume address.
  __ la(S1, resume);

  Push(S1);

  // Push frame pointer.
  Push(A3);

  // Update top in the stack. Ugh. Complicated.
  __ lw(S1, Address(S0, Process::kCoroutineOffset));
  __ lw(S1, Address(S1, Coroutine::kStackOffset - HeapObject::kTag));
  __ subu(S2, S2, S1);
  __ addiu(S2, S2, Immediate(-1 * (Stack::kSize - HeapObject::kTag)));
  __ srl(S2, S2, Immediate(1));
  __ sw(S2, Address(S1, Stack::kTopOffset - HeapObject::kTag));
}

void InterpreterGeneratorMIPS::RestoreState() {
  // Load the current stack pointer into S2.
  __ lw(S2, Address(S0, Process::kCoroutineOffset));
  __ lw(S2, Address(S2, Coroutine::kStackOffset - HeapObject::kTag));
  __ lw(S1, Address(S2, Stack::kTopOffset - HeapObject::kTag));
  __ addiu(S2, S2, Immediate(Stack::kSize - HeapObject::kTag));
  ShiftAdd(S2, S2, S1, TIMES_2);

  // Load constants into registers.
  __ lw(S6, Address(S0, Process::kProgramOffset));
  __ lw(S4, Address(S6, Program::kNullObjectOffset));
  __ la(S7, "Interpret_DispatchTable");

  // Pop and store frame pointer.
  Pop(S1);
  StoreFramePointer(S1);
  // Set the bytecode pointer from the stack.
  __ lw(S1, Address(S1, -kWordSize));

  // Pop and branch to resume address.
  Pop(RA);

  __ Jr(RA);
}

void InterpreterGeneratorMIPS::ShiftAddJump(Register reg1,
                                            Register reg2,
                                            int imm) {
  if (imm != 0) {
    __ sll(T0, reg2, Immediate(imm));
    __ addu(T1, reg1, T0);
  } else {
    __ addu(T1, reg1, reg2);
  }
  __ lw(T9, Address(T1, 0));
  __ Jr(T9);
}

void InterpreterGeneratorMIPS::ShiftAddLoad(Register reg1,
                                            Register reg2,
                                            Register reg3,
                                            int imm) {
  if (imm != 0) {
    __ sll(T0, reg3, Immediate(imm));
    __ addu(T1, reg2, T0);
  } else {
    __ addu(T1, reg2, reg3);
  }
  __ lw(reg1, Address(T1, 0));
}


void InterpreterGeneratorMIPS::ShiftAddStore(Register reg1,
                                             Register reg2,
                                             Register reg3,
                                             int imm) {
  if (imm != 0) {
    __ sll(T0, reg3, Immediate(imm));
    __ addu(T1, reg2, T0);
  } else {
    __ addu(T1, reg2, reg3);
  }
  __ sw(reg1, Address(T1, 0));
}

void InterpreterGeneratorMIPS::ShiftAdd(Register reg1,
                                        Register reg2,
                                        Register reg3,
                                        int imm) {
  if (imm != 0) {
    __ sll(T0, reg3, Immediate(imm));
    __ addu(reg1, reg2, T0);
  } else {
    __ addu(reg1, reg2, reg3);
  }
}

void InterpreterGeneratorMIPS::ShiftSub(Register reg1,
                                        Register reg2,
                                        Register reg3,
                                        int imm) {
  if (imm != 0) {
    __ sll(T0, reg3, Immediate(imm));
    __ subu(reg1, reg2, T0);
  } else {
    __ subu(reg1, reg2, reg3);
  }
}

void InterpreterGeneratorMIPS::ShiftRightAdd(Register reg1,
                                             Register reg2,
                                             Register reg3,
                                             int imm) {
  if (imm != 0) {
    __ srl(T0, reg3, Immediate(imm));
    __ addu(reg1, reg2, T0);
  } else {
    __ addu(reg1, reg2, reg3);
  }
}

void InterpreterGeneratorMIPS::PrepareStack() {
  // Reserve 4 words for function arguments and push GP.
  // The stack pointer must always be double-word aligned, so make room for
  // 6 words instead of 5.
  __ addiu(SP, SP, Immediate(-6 * kWordSize));
  __ sw(GP, Address(SP, 5 * kWordSize));
}

void InterpreterGeneratorMIPS::RestoreStack() {
  __ lw(GP, Address(SP, 5 * kWordSize));
  __ addiu(SP, SP, Immediate(6 * kWordSize));
}
}  // namespace dartino
#endif  // defined DARTINO_TARGET_MIPS
