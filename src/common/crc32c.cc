// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "include/crc32c.h"

#include "arch/probe.h"
#include "arch/intel.h"
#include "arch/arm.h"
#include "arch/ppc.h"
#include "common/sctp_crc32.h"
#include "common/crc32c_intel_baseline.h"
#include "common/crc32c_intel_fast.h"
#include "common/crc32c_aarch64.h"
#include "common/crc32c_ppc.h"

/*
 * choose best implementation based on the CPU architecture.
 */
ceph_crc32c_func_t ceph_choose_crc32(void)
{
  // make sure we've probed cpu features; this might depend on the
  // link order of this file relative to arch/probe.cc.
  ceph_arch_probe();

  // if the CPU supports it, *and* the fast version is compiled in,
  // use that.
#if defined(__i386__) || defined(__x86_64__)
  if (ceph_arch_intel_sse42 && ceph_crc32c_intel_fast_exists()) {
    return ceph_crc32c_intel_fast;
  }
#elif defined(__arm__) || defined(__aarch64__)
  if (ceph_arch_aarch64_crc32){
    return ceph_crc32c_aarch64;
  }
#elif defined(__powerpc__) || defined(__ppc__)
  if (ceph_arch_ppc_crc32) {
    return ceph_crc32c_ppc;
  }
#endif
  // default
  return ceph_crc32c_sctp;
}

/*
 * static global
 *
 * This is a bit of a no-no for shared libraries, but we don't care.
 * It is effectively constant for the executing process as the value
 * depends on the CPU architecture.
 *
 * We initialize it during program init using the magic of C++.
 */
ceph_crc32c_func_t ceph_crc32c_func = ceph_choose_crc32();

