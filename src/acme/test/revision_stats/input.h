/*
    This file is generated from version {{revision}}.
*/

#pragma ACME revision * echo v2018.10
#pragma ACME revision acme/test/revision cat input.h | wc -c
#pragma ACME stats wc wc -l
#pragma ACME stats files ls dummy | wc -l

// This particular file has exactly {{revision:acme/test/revision}} bytes,
// while there's {{stats:files}} more processed file with {{stats:wc}} lines.
