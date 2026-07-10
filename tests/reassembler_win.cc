#include "random.hh"
#include "reassembler_test_harness.hh"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <iostream>
#include <tuple>
#include <vector>

using namespace std;

static constexpr size_t NREPS = 32;
static constexpr size_t NSEGS = 128;
static constexpr size_t MAX_SEG_LEN = 2048;

int main()
{
  try {
    auto rd = get_random_engine();

    // overlapping segments
    for ( unsigned rep_no = 0; rep_no < NREPS; ++rep_no ) {
      ReassemblerTestHarness sr { "win test " + to_string( rep_no ), NSEGS * MAX_SEG_LEN };

      vector<tuple<size_t, size_t>> seq_size;
      size_t offset = 0;
      for ( unsigned i = 0; i < NSEGS; ++i ) {
        const size_t size = 1 + ( rd() % ( MAX_SEG_LEN - 1 ) );
        const size_t offs = min( offset, 1 + ( static_cast<size_t>( rd() ) % 1023 ) );
        seq_size.emplace_back( offset - offs, size + offs );
        offset += size;
      }
      shuffle( seq_size.begin(), seq_size.end(), rd );

      string d( offset, 0 );
      generate( d.begin(), d.end(), [&] { return rd(); } );

      for ( auto [off, sz] : seq_size ) {
        sr.execute( Insert { d.substr( off, sz ), off }.is_last( off + sz == offset ) );
      }

      try {
        sr.execute( ReadAll { d } );
      } catch ( const exception& ) {
        ByteStream bs2( NSEGS * MAX_SEG_LEN );
        Reassembler r2;
        for ( auto [off2, sz2] : seq_size )
          r2.insert( off2, d.substr( off2, sz2 ), off2 + sz2 == offset, bs2.writer() );
        string actual;
        read( bs2.reader(), d.size(), actual );
        size_t pos = 0;
        while ( pos < actual.size() && pos < d.size() && actual[pos] == d[pos] )
          pos++;
        cerr << "rep " << rep_no << ": exp=" << d.size() << " got=" << actual.size()
             << " mismatch @" << pos << endl;
        throw;
      }
    }
  } catch ( const exception& e ) {
    cerr << "Exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
