#ifndef CERT_H_
#define CERT_H_
unsigned char example_crt_DER[] = {
  0x30, 0x82, 0x02, 0x18, 0x30, 0x82, 0x01, 0x81, 0x02, 0x01, 0x02, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x30, 0x54, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
  0x06, 0x13, 0x02, 0x44, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x08, 0x0c, 0x02, 0x42, 0x45, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03,
  0x55, 0x04, 0x07, 0x0c, 0x06, 0x42, 0x65, 0x72, 0x6c, 0x69, 0x6e, 0x31,
  0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x4d, 0x79,
  0x43, 0x6f, 0x6d, 0x70, 0x61, 0x6e, 0x79, 0x31, 0x13, 0x30, 0x11, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x0c, 0x0a, 0x6d, 0x79, 0x63, 0x61, 0x2e, 0x6c,
  0x6f, 0x63, 0x61, 0x6c, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x31, 0x30, 0x35,
  0x31, 0x33, 0x32, 0x31, 0x33, 0x38, 0x32, 0x34, 0x5a, 0x17, 0x0d, 0x33,
  0x31, 0x30, 0x35, 0x31, 0x31, 0x32, 0x31, 0x33, 0x38, 0x32, 0x34, 0x5a,
  0x30, 0x55, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
  0x02, 0x44, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08,
  0x0c, 0x02, 0x42, 0x45, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04,
  0x07, 0x0c, 0x06, 0x42, 0x65, 0x72, 0x6c, 0x69, 0x6e, 0x31, 0x12, 0x30,
  0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x4d, 0x79, 0x43, 0x6f,
  0x6d, 0x70, 0x61, 0x6e, 0x79, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55,
  0x04, 0x03, 0x0c, 0x0b, 0x65, 0x73, 0x70, 0x33, 0x32, 0x2e, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x30, 0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 0x8d,
  0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xcd, 0xf2, 0xac, 0x81,
  0x13, 0x6e, 0xf6, 0x6a, 0x5e, 0x96, 0xe9, 0xa4, 0x4a, 0xc6, 0xcf, 0x1a,
  0x5d, 0x99, 0x90, 0x4d, 0x4a, 0xdd, 0x5e, 0x5c, 0xb3, 0x10, 0xaf, 0xb7,
  0x18, 0x01, 0x84, 0xec, 0xce, 0x93, 0xab, 0x6a, 0xb5, 0xd2, 0xa5, 0x75,
  0x0d, 0xb8, 0x2c, 0x48, 0x43, 0xaf, 0x1c, 0xeb, 0x02, 0xff, 0x30, 0xfe,
  0x6e, 0xe0, 0x45, 0xff, 0xa7, 0x7f, 0xee, 0x4a, 0x9b, 0xc5, 0xbb, 0x98,
  0x28, 0x61, 0xab, 0x46, 0x60, 0x0b, 0x5d, 0xc6, 0xf7, 0x18, 0xbc, 0xf9,
  0xbe, 0x67, 0x2f, 0xeb, 0x24, 0xfb, 0x47, 0x73, 0x2b, 0x73, 0xa3, 0x14,
  0xae, 0x02, 0x2a, 0xcf, 0x91, 0xf5, 0x56, 0x63, 0x76, 0x0c, 0x43, 0x11,
  0x4b, 0x65, 0xef, 0xcc, 0xd0, 0x94, 0x35, 0xc4, 0x33, 0xb8, 0xfb, 0xfd,
  0x19, 0x8e, 0x89, 0xde, 0xd0, 0x80, 0xf9, 0xd2, 0xea, 0xa0, 0xa3, 0x6b,
  0x4a, 0x0b, 0x33, 0xf3, 0x02, 0x03, 0x01, 0x00, 0x01, 0x30, 0x0d, 0x06,
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00,
  0x03, 0x81, 0x81, 0x00, 0xaa, 0xee, 0x19, 0xe7, 0xc7, 0x75, 0x99, 0x1e,
  0xaa, 0x6c, 0x8b, 0x1c, 0xba, 0xfe, 0xd2, 0x62, 0x69, 0x70, 0x29, 0xb5,
  0x7b, 0x69, 0x90, 0xb9, 0x5a, 0x90, 0xb4, 0x21, 0x55, 0x56, 0x13, 0xdf,
  0x99, 0x07, 0xe4, 0x76, 0x47, 0xc0, 0x0d, 0x38, 0x8f, 0x43, 0xcd, 0x0f,
  0xe5, 0xd6, 0x50, 0xdb, 0xd6, 0x18, 0xc8, 0x57, 0x97, 0x49, 0x7a, 0x3a,
  0x4e, 0x77, 0xad, 0xfe, 0x92, 0xca, 0xa8, 0x09, 0xa0, 0xa4, 0x13, 0xfa,
  0xd7, 0x75, 0x59, 0xcc, 0xd3, 0x52, 0x45, 0x14, 0x41, 0xff, 0x6c, 0x06,
  0x34, 0x9a, 0x53, 0x41, 0x19, 0x6b, 0x54, 0x97, 0xe6, 0x4a, 0xe6, 0xd5,
  0xe1, 0xbe, 0x8d, 0xae, 0xad, 0x3a, 0x51, 0x93, 0x8e, 0xc6, 0x9a, 0xe9,
  0x8d, 0xf3, 0x1a, 0xf6, 0x29, 0x70, 0x3e, 0x52, 0xb5, 0x03, 0xc9, 0x7a,
  0x51, 0x87, 0x7f, 0x19, 0x57, 0x42, 0x8a, 0xef, 0xc3, 0xf3, 0xac, 0xe5
};
unsigned int example_crt_DER_len = 540;
#endif
