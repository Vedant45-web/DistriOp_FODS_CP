#pragma once
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <functional>
#include <cmath>
#include <cstddef>

// Educational implementations used by the DistriOpt scheduling core.
template<class T> class FODSArray {
    T* data_{nullptr}; size_t size_{0},capacity_{0};
    void grow(){size_t next=capacity_?capacity_*2:4;T* fresh=new T[next];for(size_t i=0;i<size_;++i)fresh[i]=data_[i];delete[] data_;data_=fresh;capacity_=next;}
public:
    FODSArray()=default;~FODSArray(){delete[] data_;}
    FODSArray(const FODSArray& other){if(other.size_){capacity_=other.size_;size_=other.size_;data_=new T[capacity_];for(size_t i=0;i<size_;++i)data_[i]=other.data_[i];}}
    FODSArray& operator=(const FODSArray& other){if(this==&other)return *this;T* fresh=other.size_?new T[other.size_]:nullptr;for(size_t i=0;i<other.size_;++i)fresh[i]=other.data_[i];delete[] data_;data_=fresh;size_=capacity_=other.size_;return *this;}
    FODSArray(FODSArray&& other)noexcept:data_(other.data_),size_(other.size_),capacity_(other.capacity_){other.data_=nullptr;other.size_=other.capacity_=0;}
    FODSArray& operator=(FODSArray&& other)noexcept{if(this!=&other){delete[] data_;data_=other.data_;size_=other.size_;capacity_=other.capacity_;other.data_=nullptr;other.size_=other.capacity_=0;}return *this;}
    void push(const T& value){if(size_==capacity_)grow();data_[size_++]=value;}
    T& at(size_t i){if(i>=size_)throw std::out_of_range("FODSArray index");return data_[i];}
    const T& at(size_t i)const{if(i>=size_)throw std::out_of_range("FODSArray index");return data_[i];}
    size_t size()const{return size_;}bool empty()const{return size_==0;}
};
template<class T> class FODSLinkedList {
    struct Node{T value;Node*next;explicit Node(const T&v):value(v),next(nullptr){}};Node*head_{nullptr};size_t size_{0};
public:
    ~FODSLinkedList(){clear();} FODSLinkedList()=default;FODSLinkedList(const FODSLinkedList&)=delete;FODSLinkedList&operator=(const FODSLinkedList&)=delete;
    void pushBack(const T&v){Node*n=new Node(v);if(!head_)head_=n;else{Node*p=head_;while(p->next)p=p->next;p->next=n;}++size_;}
    bool removeFirst(const T&v){Node*p=head_,*prev=nullptr;while(p){if(p->value==v){if(prev)prev->next=p->next;else head_=p->next;delete p;--size_;return true;}prev=p;p=p->next;}return false;}
    bool contains(const T&v)const{for(Node*p=head_;p;p=p->next)if(p->value==v)return true;return false;}size_t size()const{return size_;}
    void clear(){while(head_){Node*n=head_->next;delete head_;head_=n;}size_=0;}
};
template<class T> class FODSStack {
    std::vector<T> data_;
public:void push(const T&v){data_.push_back(v);}T pop(){if(data_.empty())throw std::underflow_error("Stack empty");T v=data_.back();data_.pop_back();return v;}bool empty()const{return data_.empty();}size_t size()const{return data_.size();}
};
template<class T> class FODSQueue {
    struct Node{T value;Node*next;explicit Node(const T&v):value(v),next(nullptr){}};Node*front_{nullptr};Node*back_{nullptr};size_t size_{0};
public:~FODSQueue(){clear();}void push(const T&v){Node*n=new Node(v);if(back_)back_->next=n;else front_=n;back_=n;++size_;}T pop(){if(!front_)throw std::underflow_error("Queue empty");Node*n=front_;T v=n->value;front_=n->next;if(!front_)back_=nullptr;delete n;--size_;return v;}bool empty()const{return front_==nullptr;}size_t size()const{return size_;}void clear(){while(front_){Node*n=front_->next;delete front_;front_=n;}back_=nullptr;size_=0;}
};
template<class T> class FODSPriorityQueue {
    struct Item{T value;int priority;};std::vector<Item> heap_;
    void up(size_t i){while(i){size_t p=(i-1)/2;if(heap_[p].priority>=heap_[i].priority)break;std::swap(heap_[p],heap_[i]);i=p;}}
    void down(size_t i){for(;;){size_t l=2*i+1,r=l+1,b=i;if(l<heap_.size()&&heap_[l].priority>heap_[b].priority)b=l;if(r<heap_.size()&&heap_[r].priority>heap_[b].priority)b=r;if(b==i)break;std::swap(heap_[i],heap_[b]);i=b;}}
public:void push(const T&v,int p){heap_.push_back({v,p});up(heap_.size()-1);}T pop(){if(heap_.empty())throw std::underflow_error("Priority queue empty");T v=heap_[0].value;heap_[0]=heap_.back();heap_.pop_back();if(!heap_.empty())down(0);return v;}bool empty()const{return heap_.empty();}size_t size()const{return heap_.size();}
};
template<class T> int linearSearch(const std::vector<T>&a,const T&key){for(size_t i=0;i<a.size();++i)if(a[i]==key)return static_cast<int>(i);return -1;}
template<class T> int binarySearch(const std::vector<T>&a,const T&key){int l=0,r=static_cast<int>(a.size())-1;while(l<=r){int m=l+(r-l)/2;if(a[m]==key)return m;if(a[m]<key)l=m+1;else r=m-1;}return -1;}
template<class T> void bubbleSort(std::vector<T>&a){for(size_t i=0;i<a.size();++i)for(size_t j=0;j+1<a.size()-i;++j)if(a[j]>a[j+1])std::swap(a[j],a[j+1]);}
template<class T> void insertionSort(std::vector<T>&a){for(size_t i=1;i<a.size();++i){T x=a[i];size_t j=i;while(j&&a[j-1]>x){a[j]=a[j-1];--j;}a[j]=x;}}
template<class T> void selectionSort(std::vector<T>&a){for(size_t i=0;i<a.size();++i){size_t m=i;for(size_t j=i+1;j<a.size();++j)if(a[j]<a[m])m=j;std::swap(a[i],a[m]);}}
template<class T> void quickSort(std::vector<T>&a,int l,int r){if(l>=r)return;T pivot=a[l+(r-l)/2];int i=l,j=r;while(i<=j){while(a[i]<pivot)++i;while(a[j]>pivot)--j;if(i<=j){std::swap(a[i],a[j]);++i;--j;}}if(l<j)quickSort(a,l,j);if(i<r)quickSort(a,i,r);}
template<class T> void mergeSort(std::vector<T>&a){if(a.size()<2)return;std::vector<T>tmp(a.size());for(size_t width=1;width<a.size();width*=2){for(size_t left=0;left<a.size();left+=2*width){size_t mid=std::min(left+width,a.size()),right=std::min(left+2*width,a.size()),i=left,j=mid,k=left;while(i<mid&&j<right)tmp[k++]=(a[i]<a[j])?a[i++]:a[j++];while(i<mid)tmp[k++]=a[i++];while(j<right)tmp[k++]=a[j++];for(size_t p=left;p<right;++p)a[p]=tmp[p];}if(width>a.size()/2)break;}}

// Additional search techniques from the FODS syllabus.
template<class T> int sentinelSearch(std::vector<T> a,const T& key){size_t n=a.size();a.push_back(key);size_t i=0;while(!(a[i]==key))++i;return i<n?static_cast<int>(i):-1;}
template<class T> int fibonacciSearch(const std::vector<T>& a,const T& key){int n=(int)a.size();int fib2=0,fib1=1,fib=fib1+fib2;while(fib<n){fib2=fib1;fib1=fib;fib=fib1+fib2;}int offset=-1;while(fib>1){int i=std::min(offset+fib2,n-1);if(a[i]<key){fib=fib1;fib1=fib2;fib2=fib-fib1;offset=i;}else if(a[i]>key){fib=fib2;fib1-=fib2;fib2=fib-fib1;}else return i;}if(fib1&&offset+1<n&&a[offset+1]==key)return offset+1;return -1;}
template<class T> int indexedSequentialSearch(const std::vector<T>& a,const T& key){if(a.empty())return -1;size_t step=static_cast<size_t>(std::sqrt((double)a.size()));if(step==0)step=1;size_t start=0,end=std::min(step,a.size());while(start<a.size()&&a[end-1]<key){start=end;if(start>=a.size())return -1;end=std::min(end+step,a.size());}for(size_t i=start;i<end;++i){if(a[i]==key)return (int)i;if(a[i]>key)break;}return -1;}
// Additional sorting techniques. Counting/radix sort accept integers; bucket sort accepts values in [0,1].
template<class T> void shellSort(std::vector<T>& a){for(size_t gap=a.size()/2;gap>0;gap/=2){for(size_t i=gap;i<a.size();++i){T temp=a[i];size_t j=i;while(j>=gap&&a[j-gap]>temp){a[j]=a[j-gap];j-=gap;}a[j]=temp;}}}
inline void countingSort(std::vector<int>& a){if(a.empty())return;auto mm=std::minmax_element(a.begin(),a.end());int lo=*mm.first,hi=*mm.second;if((long long)hi-lo>10000000)throw std::length_error("Counting sort range too large");std::vector<size_t>count((size_t)((long long)hi-lo+1),0);for(int v:a)++count[(size_t)((long long)v-lo)];size_t k=0;for(size_t i=0;i<count.size();++i)for(size_t c=0;c<count[i];++c)a[k++]=(int)((long long)i+lo);}
inline void radixSort(std::vector<int>& a){if(a.empty())return;if(std::any_of(a.begin(),a.end(),[](int v){return v<0;}))throw std::invalid_argument("Radix sort expects non-negative integers");int maxv=*std::max_element(a.begin(),a.end());std::vector<int>out(a.size());for(long long exp=1;maxv/exp>0;exp*=10){int count[10]={0};for(int v:a)++count[(v/exp)%10];for(int i=1;i<10;++i)count[i]+=count[i-1];for(int i=(int)a.size()-1;i>=0;--i){int digit=(int)((a[i]/exp)%10);out[--count[digit]]=a[i];}a.swap(out);if(exp>maxv/10)break;}}
inline void bucketSort(std::vector<double>& a){if(a.empty())return;for(double v:a)if(v<0.0||v>1.0)throw std::invalid_argument("Bucket sort expects values in [0,1]");std::vector<std::vector<double>>b(a.size());for(double v:a){size_t i=std::min(a.size()-1,(size_t)(v*a.size()));b[i].push_back(v);}size_t k=0;for(auto&bucket:b){insertionSort(bucket);for(double v:bucket)a[k++]=v;}}
