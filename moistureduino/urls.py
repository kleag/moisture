from django.urls import path
from rest_framework.urlpatterns import format_suffix_patterns
from moistureduino import views

urlpatterns = [
    path('', views.api_root),
    path('entries/', views.EntryList.as_view(),
         name='entry-list'),
    path('entries/<int:pk>/', views.EntryDetail.as_view(),
         name='entry-detail'),
    path('entries/<int:pk>/highlight/', views.EntryHighlight.as_view(),
         name='entry-highlight'),
    path('users/', views.UserList.as_view(),
         name='user-list'),
    path('users/<int:pk>/', views.UserDetail.as_view(),
         name='user-detail'),
]

urlpatterns = format_suffix_patterns(urlpatterns)


